const SUPABASE_URL = "https://witvuhdpcpfolevfjapq.supabase.co";
const SUPABASE_KEY = "sb_publishable_sbNdNAgtIPJu5M8eq8u4uQ_yZfwFBnN";

const WINDOW_MS = 300_000;
const MAX_READING_GAP_MS = 5_000;
const SENSOR_IDS = [1, 2];
const SENSOR_COLORS = ["#dc2626", "#2563eb"];

const client = supabase.createClient(SUPABASE_URL, SUPABASE_KEY);

const element = (id) => document.getElementById(id);
const formatTime = (time) => new Date(time).toLocaleTimeString();
const displayTemperature = (celsius) =>
    unit === "F" ? celsius * 9 / 5 + 32 : celsius;

const readings = new Map();
const latest = new Map();
const alertThresholds = new Map();

let refreshing = false;
let realtimeConnected = false;
let unit = "C";


/* -------------------------------------------------------------------------- */
/* Chart setup                                                                */
/* -------------------------------------------------------------------------- */

function createSensorDataset(id, index) {
    const color = SENSOR_COLORS[index];

    return {
        label: `Temperature Sensor ${id}`,
        data: [],
        borderColor: color,
        backgroundColor: color,
        borderWidth: 2,
        pointRadius: 3,
        pointHoverRadius: 5,
        cubicInterpolationMode: "monotone",

        // Break the graph line when readings are more than 5 seconds apart.
        segment: {
            borderColor: (ctx) => {
                const gap = ctx.p1.parsed.x - ctx.p0.parsed.x;
                return gap > MAX_READING_GAP_MS ? "transparent" : color;
            }
        }
    };
}

function createThresholdDataset(sensor, type, color) {
    return {
        label: `Sensor ${sensor} ${type === "max" ? "Max" : "Min"} Alert`,
        thresholdSensor: sensor,
        thresholdType: type,
        data: [],
        borderColor: color,
        borderWidth: 2,
        borderDash: type === "max" ? [8, 6] : [2, 4],
        pointRadius: 0,
        pointHoverRadius: 0,
        tension: 0
    };
}

const sensorDatasets = SENSOR_IDS.map(createSensorDataset);

const thresholdDatasets = SENSOR_IDS.flatMap((sensor, index) => [
    createThresholdDataset(sensor, "max", SENSOR_COLORS[index]),
    createThresholdDataset(sensor, "min", SENSOR_COLORS[index])
]);

const chart = new Chart(element("temperature-chart"), {
    type: "line",

    data: {
        datasets: [
            ...sensorDatasets,
            ...thresholdDatasets
        ]
    },

    options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        parsing: false,

        plugins: {
            tooltip: {
                callbacks: {
                    title: (items) => formatTime(items[0].parsed.x),
                    label: (item) =>
                        `${item.dataset.label}: ${item.parsed.y.toFixed(1)} °${unit}`
                }
            }
        },

        scales: {
            x: {
                type: "linear",
                min: Date.now() - WINDOW_MS,
                max: Date.now(),

                ticks: {
                    callback: formatTime,
                    maxTicksLimit: 7
                },

                title: {
                    display: true,
                    text: "Time"
                }
            },

            y: {
                position: "right",
                min: 10,
                max: 50,

                title: {
                    display: true,
                    text: "Temperature (°C)"
                }
            }
        }
    }
});


/* -------------------------------------------------------------------------- */
/* Temperature display                                                        */
/* -------------------------------------------------------------------------- */

function setTemperatureUnit(value) {
    unit = value === "F" ? "F" : "C";
    render();
}

function addReadings(rows) {
    for (const row of rows) {
        const sensor = Number(row.sensor_id);
        const x = Date.parse(row.recorded_at);
        const y = Number(row.temperature);

        if (
            !SENSOR_IDS.includes(sensor) ||
            !Number.isFinite(x) ||
            row.temperature == null ||
            row.temperature === "" ||
            !Number.isFinite(y)
        ) {
            continue;
        }

        // History, polling, and realtime may deliver the same row.
        readings.set(
            row.id ?? `${sensor}:${row.recorded_at}`,
            { sensor, x, y }
        );

        // Keep track of the latest reading for each sensor.
        if (!latest.has(sensor) || x >= latest.get(sensor).x) {
            latest.set(sensor, { x, y });
        }
    }

    render();
}

function render() {
    const now = Date.now();
    const cutoff = now - WINDOW_MS;

    // Remove readings outside the 300-second window.
    for (const [id, point] of readings) {
        if (point.x < cutoff) readings.delete(id);
    }

    const points = [...readings.values()].sort((a, b) => a.x - b.x);

    // Update sensor data.
    sensorDatasets.forEach((dataset, index) => {
        const sensor = SENSOR_IDS[index];

        dataset.data = points
            .filter((point) => point.sensor === sensor && point.x <= now)
            .map((point) => ({
                x: point.x,
                y: displayTemperature(point.y)
            }));
    });

    // Update min/max threshold lines.
    thresholdDatasets.forEach((dataset) => {
        const thresholds = alertThresholds.get(dataset.thresholdSensor);
        const value = thresholds?.[dataset.thresholdType];

        if (!Number.isFinite(value)) {
            dataset.data = [];
            return;
        }

        const y = displayTemperature(value);

        dataset.data = [
            { x: cutoff, y },
            { x: now, y }
        ];
    });

    // Update chart range and units.
    chart.options.scales.x.min = cutoff;
    chart.options.scales.x.max = now;
    chart.options.scales.y.min = displayTemperature(10);
    chart.options.scales.y.max = displayTemperature(50);
    chart.options.scales.y.title.text = `Temperature (°${unit})`;

    chart.update("none");

    // Update current-temperature displays.
    for (const sensor of SENSOR_IDS) {
        const point = latest.get(sensor);

        element(`temperature-${sensor}`).textContent = point
            ? `${displayTemperature(point.y).toFixed(1)} °${unit}`
            : `-- °${unit}`;

        element(`last-update-${sensor}`).textContent = point
            ? `Last update: ${formatTime(point.x)}${point.x < cutoff ? " (stale)" : ""}`
            : "Waiting for data...";
    }
}


/* -------------------------------------------------------------------------- */
/* Temperature history                                                        */
/* -------------------------------------------------------------------------- */

async function loadTemperatureHistory() {
    const since = new Date(Date.now() - WINDOW_MS).toISOString();
    const until = new Date().toISOString();
    const pageSize = 500;

    // Paginate so API row limits do not truncate either sensor.
    for (let offset = 0; ; offset += pageSize) {
        const { data, error } = await client
            .from("temperature_readings")
            .select("id,sensor_id,temperature,recorded_at")
            .in("sensor_id", SENSOR_IDS)
            .gte("recorded_at", since)
            .lte("recorded_at", until)
            .order("recorded_at")
            .order("id")
            .range(offset, offset + pageSize - 1);

        if (error) throw error;

        addReadings(data);

        if (data.length < pageSize) break;
    }
}


/* -------------------------------------------------------------------------- */
/* Sensor state                                                               */
/* -------------------------------------------------------------------------- */

function updateSensorStatus(row) {
    const sensor = Number(row.sensor_id);

    if (!SENSOR_IDS.includes(sensor)) return;

    element(`sensor-${sensor}-status`).textContent =
        row.enabled == null
            ? "Unknown"
            : row.enabled
                ? "🟢 ON"
                : "🔴 OFF";
}

async function loadSensorState() {
    const { data, error } = await client
        .from("sensor_state")
        .select("sensor_id,enabled")
        .in("sensor_id", SENSOR_IDS);

    if (error) {
        console.error("Could not load sensor state:", error);
        return;
    }

    data.forEach(updateSensorStatus);
}

async function setSensor(sensor, enabled) {
    sensor = Number(sensor);

    if (!SENSOR_IDS.includes(sensor)) return;

    const status = element("command-status");

    status.textContent =
        `Setting Sensor ${sensor} ${enabled ? "ON" : "OFF"}...`;

    const { error } = await client
        .from("sensor_state")
        .update({ enabled })
        .eq("sensor_id", sensor);

    if (error) {
        status.textContent = `❌ Could not update Sensor ${sensor}`;
        console.error("Sensor state update failed:", error);
        return;
    }

    status.textContent =
        `✓ Sensor ${sensor} set to ${enabled ? "ON" : "OFF"}; ` +
        "awaiting device confirmation";

    updateSensorStatus({
        sensor_id: sensor,
        enabled
    });
}


/* -------------------------------------------------------------------------- */
/* Alert settings                                                             */
/* -------------------------------------------------------------------------- */

async function loadAlertSettings() {
    const { data, error } = await client
        .from("alert_settings")
        .select(
            "sensor_id,max_temp,min_temp,contact_email,high_message,low_message"
        )
        .in("sensor_id", SENSOR_IDS);

    if (error) {
        console.error("Could not load alert settings:", error);
        return;
    }

    for (const row of data) {
        const sensor = Number(row.sensor_id);

        element(`max-temp-${sensor}`).value = row.max_temp ?? "";
        element(`min-temp-${sensor}`).value = row.min_temp ?? "";
        element(`contact-email-${sensor}`).value = row.contact_email ?? "";
        element(`high-message-${sensor}`).value = row.high_message ?? "";
        element(`low-message-${sensor}`).value = row.low_message ?? "";

        // Thresholds are stored internally in Celsius.
        alertThresholds.set(sensor, {
            max: row.max_temp == null ? NaN : Number(row.max_temp),
            min: row.min_temp == null ? NaN : Number(row.min_temp)
        });
    }

    render();
}

async function saveAlertSettings(sensor) {
    sensor = Number(sensor);

    if (!SENSOR_IDS.includes(sensor)) return;

    const status = element(`alert-status-${sensor}`);
    const maxTemp = Number(element(`max-temp-${sensor}`).value);
    const minTemp = Number(element(`min-temp-${sensor}`).value);

    status.textContent = "Saving...";

    if (
        !Number.isFinite(maxTemp) ||
        !Number.isFinite(minTemp) ||
        maxTemp <= minTemp
    ) {
        status.textContent =
            "❌ Max temperature must be greater than min temperature";
        return;
    }

    const { error } = await client
        .from("alert_settings")
        .update({
            max_temp: maxTemp,
            min_temp: minTemp,
            contact_email: element(`contact-email-${sensor}`).value,
            high_message: element(`high-message-${sensor}`).value,
            low_message: element(`low-message-${sensor}`).value
        })
        .eq("sensor_id", sensor);

    if (error) {
        status.textContent = "❌ Could not save alert settings";
        console.error("Alert settings save failed:", error);
        return;
    }

    status.textContent = "✓ Alert settings saved";

    // Update the chart immediately without waiting for another DB request.
    alertThresholds.set(sensor, {
        max: maxTemp,
        min: minTemp
    });

    render();
}


/* -------------------------------------------------------------------------- */
/* Refresh / connection handling                                              */
/* -------------------------------------------------------------------------- */

async function refresh() {
    if (refreshing) return;

    refreshing = true;

    try {
        await loadTemperatureHistory();

        element("connection-status").textContent =
            realtimeConnected
                ? "🟢 Live"
                : "🟡 Live via polling";
    } catch (error) {
        console.error("Could not load temperatures:", error);

        element("connection-status").textContent =
            "🔴 Refresh failed — retrying";
    } finally {
        refreshing = false;
    }

    await loadSensorState();
}


/* -------------------------------------------------------------------------- */
/* Supabase realtime                                                          */
/* -------------------------------------------------------------------------- */

client
    .channel("temperature-monitor")

    .on(
        "postgres_changes",
        {
            event: "INSERT",
            schema: "public",
            table: "temperature_readings"
        },
        ({ new: row }) => addReadings([row])
    )

    .on(
        "postgres_changes",
        {
            event: "*",
            schema: "public",
            table: "sensor_state"
        },
        ({ new: row }) => updateSensorStatus(row)
    )

    .subscribe((status) => {
        realtimeConnected = status === "SUBSCRIBED";

        element("connection-status").textContent =
            realtimeConnected
                ? "🟢 Live"
                : "🟡 Connecting — polling enabled";

        if (realtimeConnected) refresh();
    });


/* -------------------------------------------------------------------------- */
/* Timers / startup                                                           */
/* -------------------------------------------------------------------------- */

// Advance the graph even when sensors stop sending readings.
setInterval(render, 250);

// Poll periodically to fill gaps after connection/realtime interruptions.
setInterval(refresh, 5000);

// Refresh when returning to the browser tab.
document.addEventListener("visibilitychange", () => {
    if (!document.hidden) {
        render();
        refresh();
    }
});

// Initial load.
refresh();
loadAlertSettings();