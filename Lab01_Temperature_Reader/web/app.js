const SUPABASE_URL = "https://witvuhdpcpfolevfjapq.supabase.co";
const SUPABASE_KEY = "sb_publishable_sbNdNAgtIPJu5M8eq8u4uQ_yZfwFBnN";
const WINDOW_MS = 300_000;
const SENSOR_IDS = [1, 2];
const client = supabase.createClient(SUPABASE_URL, SUPABASE_KEY);
const element = (id) => document.getElementById(id);
const formatTime = (time) => new Date(time).toLocaleTimeString();
const readings = new Map();
const latest = new Map();
let refreshing = false;
let realtimeConnected = false;
let unit = "C";
const displayTemperature = (celsius) => unit === "F" ? celsius * 9 / 5 + 32 : celsius;

function setTemperatureUnit(value) {
    unit = value === "F" ? "F" : "C";
    render();
}

const chart = new Chart(element("temperature-chart"), {
    type: "line",
    data: {
        datasets: SENSOR_IDS.map((id, index) => ({
            label: `Temperature Sensor ${id}`,
            data: [],
            borderColor: ["#dc2626", "#2563eb"][index],
            backgroundColor: ["#dc2626", "#2563eb"][index],
            borderWidth: 2,
            pointRadius: 3,
            pointHoverRadius: 5,
            cubicInterpolationMode: "monotone"
        }))
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
                    label: (item) => `${item.dataset.label}: ${item.parsed.y.toFixed(1)} °${unit}`
                }
            }
        },
        scales: {
            x: {
                type: "linear",
                min: Date.now() - WINDOW_MS,
                max: Date.now(),
                ticks: { callback: formatTime, maxTicksLimit: 7 },
                title: { display: true, text: "Time" }
            },
            y: {
                position: "right",
                min: 10,
                max: 50,
                title: { display: true, text: "Temperature (°C)" }
            }
        }
    }
});

function addReadings(rows) {
    for (const row of rows) {
        const sensor = Number(row.sensor_id);
        const x = Date.parse(row.recorded_at);
        const y = Number(row.temperature);
        if (!SENSOR_IDS.includes(sensor) || !Number.isFinite(x) ||
            row.temperature == null || row.temperature === "" || !Number.isFinite(y)) continue;

        // History, polling, and realtime can deliver the same row.
        readings.set(row.id ?? `${sensor}:${row.recorded_at}`, { sensor, x, y });
        if (!latest.has(sensor) || x >= latest.get(sensor).x) latest.set(sensor, { x, y });
    }
    render();
}

function render() {
    const now = Date.now();
    const cutoff = now - WINDOW_MS;
    for (const [id, point] of readings) {
        if (point.x < cutoff) readings.delete(id);
    }
    const points = [...readings.values()].sort((a, b) => a.x - b.x);
    chart.data.datasets.forEach((dataset, index) => {
        dataset.data = points
            .filter((point) => point.sensor === SENSOR_IDS[index] && point.x <= now)
            .map((point) => ({ x: point.x, y: displayTemperature(point.y) }));
    });
    chart.options.scales.x.min = cutoff;
    chart.options.scales.x.max = now;
    chart.options.scales.y.min = displayTemperature(10);
    chart.options.scales.y.max = displayTemperature(50);
    chart.options.scales.y.title.text = `Temperature (°${unit})`;
    chart.update("none");

    for (const sensor of SENSOR_IDS) {
        const point = latest.get(sensor);
        element(`temperature-${sensor}`).textContent = point
            ? `${displayTemperature(point.y).toFixed(1)} °${unit}` : `-- °${unit}`;
        element(`last-update-${sensor}`).textContent = point
            ? `Last update: ${formatTime(point.x)}${point.x < cutoff ? " (stale)" : ""}`
            : "Waiting for data...";
    }
}

async function loadTemperatureHistory() {
    // Paginate the time window so an API row limit never truncates either sensor.
    const since = new Date(Date.now() - WINDOW_MS).toISOString();
    const until = new Date().toISOString();
    const pageSize = 500;
    for (let offset = 0; ; offset += pageSize) {
        const { data, error } = await client.from("temperature_readings")
            .select("id,sensor_id,temperature,recorded_at")
            .in("sensor_id", SENSOR_IDS)
            .gte("recorded_at", since).lte("recorded_at", until)
            .order("recorded_at").order("id")
            .range(offset, offset + pageSize - 1);
        if (error) throw error;
        addReadings(data);
        if (data.length < pageSize) break;
    }
}

function updateSensorStatus(row) {
    if (!SENSOR_IDS.includes(Number(row.sensor_id))) return;
    element(`sensor-${row.sensor_id}-status`).textContent =
        row.enabled == null ? "Unknown" : row.enabled ? "🟢 ON" : "🔴 OFF";
}

async function loadSensorState() {
    const { data, error } = await client.from("sensor_state")
        .select("sensor_id,enabled").in("sensor_id", SENSOR_IDS);
    if (error) {
        console.error("Could not load sensor state:", error);
        return;
    }
    data.forEach(updateSensorStatus);
}

async function refresh() {
    if (refreshing) return;
    refreshing = true;
    try {
        await loadTemperatureHistory();
        element("connection-status").textContent = realtimeConnected
            ? "🟢 Live" : "🟡 Live via polling";
    } catch (error) {
        console.error("Could not load temperatures:", error);
        element("connection-status").textContent = "🔴 Refresh failed — retrying";
    } finally {
        refreshing = false;
    }
    await loadSensorState();
}

async function setSensor(sensor, enabled) {
    if (!SENSOR_IDS.includes(Number(sensor))) return;

    element("command-status").textContent =
        `Setting Sensor ${sensor} ${enabled ? "ON" : "OFF"}...`;

    const { error } = await client
        .from("sensor_state")
        .update({ enabled })
        .eq("sensor_id", sensor);

    if (error) {
        element("command-status").textContent =
            `❌ Could not update Sensor ${sensor}`;

        console.error(
            "Sensor state update failed:",
            error
        );

        return;
    }

    element("command-status").textContent =
        `✓ Sensor ${sensor} set to ${enabled ? "ON" : "OFF"}; awaiting device confirmation`;

    updateSensorStatus({
        sensor_id: sensor,
        enabled
    });
}

client.channel("temperature-monitor")
    .on("postgres_changes", {
        event: "INSERT", schema: "public", table: "temperature_readings"
    }, ({ new: row }) => addReadings([row]))
    .on("postgres_changes", {
        event: "*", schema: "public", table: "sensor_state"
    }, ({ new: row }) => updateSensorStatus(row))
    .subscribe((status) => {
        realtimeConnected = status === "SUBSCRIBED";
        element("connection-status").textContent = realtimeConnected
            ? "🟢 Live" : "🟡 Connecting — polling enabled";
        if (realtimeConnected) refresh();
    });

// Advance the clock even when sensors stop sending data; polling fills reconnect gaps.
setInterval(render, 250);
setInterval(refresh, 5000);
document.addEventListener("visibilitychange", () => {
    if (!document.hidden) {
        render();
        refresh();
    }
});
refresh();
