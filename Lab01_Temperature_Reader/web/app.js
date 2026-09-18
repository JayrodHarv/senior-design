const SUPABASE_URL = "https://witvuhdpcpfolevfjapq.supabase.co";
const SUPABASE_KEY = "sb_publishable_sbNdNAgtIPJu5M8eq8u4uQ_yZfwFBnN";

const WINDOW_MS = 300_000;
const MAX_READING_GAP_MS = 5_000;
const SENSOR_DISCONNECT_TIMEOUT_MS = 5_000;
const TEMPERATURE_POLL_INTERVAL_MS = 500;
const TEMPERATURE_POLL_OVERLAP_MS = 2_000;
const TEMPERATURE_POLL_BATCH_SIZE = 100;
const SENSOR_IDS = [1, 2];
const SENSOR_COLORS = ["#dc2626", "#2563eb"];
const CHART_MIN_C = 10;
const CHART_MAX_C = 50;
const OUT_OF_RANGE_COLOR = "#ea580c";

const client = supabase.createClient(
    SUPABASE_URL,
    SUPABASE_KEY
);

const element = (id) =>
    document.getElementById(id);

const formatTime = (time) =>
    new Date(time).toLocaleTimeString();

const displayTemperature = (celsius) =>
    unit === "F"
        ? celsius * 9 / 5 + 32
        : celsius;

const readings = new Map();
const latest = new Map();
const sensorEnabled = new Map();
const waitingSince = new Map(SENSOR_IDS.map((sensor) => [sensor, Date.now()]));
const alertThresholds = new Map();

let refreshing = false;
let temperaturePollInProgress = false;
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
        pointRadius: (ctx) => ctx.raw?.outOfRange ? 3 : 1,
        pointBackgroundColor: (ctx) =>
            ctx.raw?.outOfRange ? OUT_OF_RANGE_COLOR : color,
        pointBorderColor: (ctx) =>
            ctx.raw?.outOfRange ? OUT_OF_RANGE_COLOR : color,
        pointHoverRadius: 5,
        cubicInterpolationMode: "monotone",

        // Break the line when readings are more than 5 seconds apart.
        segment: {
            borderColor: (ctx) => {
                const gap =
                    ctx.p1.parsed.x -
                    ctx.p0.parsed.x;

                return gap > MAX_READING_GAP_MS
                    ? "transparent"
                    : ctx.p0.raw.outOfRange || ctx.p1.raw.outOfRange
                        ? OUT_OF_RANGE_COLOR
                        : color;
            }
        }
    };
}

function createThresholdDataset(
    sensor,
    type,
    color
) {
    return {
        label:
            `Sensor ${sensor} ` +
            `${type === "max" ? "Max" : "Min"} Alert`,

        thresholdSensor: sensor,
        thresholdType: type,
        data: [],
        borderColor: color,
        borderWidth: 2,
        borderDash:
            type === "max"
                ? [8, 6]
                : [2, 4],

        pointRadius: 0,
        pointHoverRadius: 0,
        tension: 0
    };
}

const sensorDatasets =
    SENSOR_IDS.map(createSensorDataset);

const thresholdDatasets =
    SENSOR_IDS.flatMap(
        (sensor, index) => [
            createThresholdDataset(
                sensor,
                "max",
                SENSOR_COLORS[index]
            ),

            createThresholdDataset(
                sensor,
                "min",
                SENSOR_COLORS[index]
            )
        ]
    );

const chart = new Chart(
    element("temperature-chart"),
    {
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
                        title: (items) =>
                            formatTime(
                                items[0].parsed.x
                            ),

                        label: (item) =>
                            `${item.dataset.label}: ` +
                            `${(item.raw.actualTemperature ?? item.parsed.y).toFixed(1)} °${unit}` +
                            (item.raw.outOfRange ? " (out of range)" : "")
                    }
                }
            },

            scales: {
                x: {
                    type: "linear",
                    min:
                        Date.now() -
                        WINDOW_MS,

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
                    min: CHART_MIN_C,
                    max: CHART_MAX_C,

                    title: {
                        display: true,
                        text: "Temperature (°C)"
                    }
                }
            }
        }
    }
);


/* -------------------------------------------------------------------------- */
/* Temperature display                                                        */
/* -------------------------------------------------------------------------- */

function setTemperatureUnit(value) {
    unit =
        value === "F"
            ? "F"
            : "C";

    render();
}

function addReadings(rows) {
    for (const row of rows) {
        const sensor =
            Number(row.sensor_id);

        const x =
            Date.parse(row.recorded_at);

        const y =
            Number(row.temperature);

        if (
            !SENSOR_IDS.includes(sensor) ||
            !Number.isFinite(x) ||
            row.temperature == null ||
            row.temperature === "" ||
            !Number.isFinite(y)
        ) {
            continue;
        }

        // Realtime, history, and polling can deliver
        // the same row. The Map removes duplicates.
        readings.set(
            row.id ??
                `${sensor}:${row.recorded_at}`,
            {
                sensor,
                x,
                y
            }
        );

        // Store the latest reading for each sensor.
        if (
            !latest.has(sensor) ||
            x >= latest.get(sensor).x
        ) {
            latest.set(
                sensor,
                {
                    x,
                    y
                }
            );
        }
    }

    render();
}

function render() {
    const now = Date.now();
    const cutoff =
        now - WINDOW_MS;

    // Remove readings older than 300 seconds.
    for (const [id, point] of readings) {
        if (point.x < cutoff) {
            readings.delete(id);
        }
    }

    const points =
        [...readings.values()]
            .sort(
                (a, b) =>
                    a.x - b.x
            );

    // Update temperature datasets.
    sensorDatasets.forEach(
        (dataset, index) => {
            const sensor =
                SENSOR_IDS[index];

            dataset.data =
                points
                    .filter(
                        (point) =>
                            point.sensor === sensor &&
                            point.x <= now
                    )
                    .map(
                        (point) => ({
                            x: point.x,
                            y:
                                displayTemperature(
                                    Math.min(CHART_MAX_C, Math.max(CHART_MIN_C, point.y))
                                ),
                            actualTemperature: displayTemperature(point.y),
                            outOfRange: point.y < CHART_MIN_C || point.y > CHART_MAX_C
                        })
                    );
        }
    );

    // Update min/max threshold lines.
    thresholdDatasets.forEach(
        (dataset) => {
            const thresholds =
                alertThresholds.get(
                    dataset.thresholdSensor
                );

            const value =
                thresholds?.[
                    dataset.thresholdType
                ];

            if (!Number.isFinite(value)) {
                dataset.data = [];
                return;
            }

            const y =
                displayTemperature(value);

            dataset.data = [
                {
                    x: cutoff,
                    y
                },
                {
                    x: now,
                    y
                }
            ];
        }
    );

    // Update chart range and units.
    chart.options.scales.x.min =
        cutoff;

    chart.options.scales.x.max =
        now;

    chart.options.scales.y.min =
        displayTemperature(CHART_MIN_C);

    chart.options.scales.y.max =
        displayTemperature(CHART_MAX_C);

    chart.options.scales.y.title.text =
        `Temperature (°${unit})`;

    chart.update("none");

    // Update current-temperature displays.
    for (const sensor of SENSOR_IDS) {
        const point =
            latest.get(sensor);

        const disabled = sensorEnabled.get(sensor) === false;
        // The firmware skips uploads for disconnected sensors. Infer a
        // disconnect when readings stop, allowing time for the first upload.
        const disconnected = !disabled &&
            now - Math.max(point?.x ?? 0, waitingSince.get(sensor)) >=
                SENSOR_DISCONNECT_TIMEOUT_MS;
        const temperature = element(
            `temperature-${sensor}`
        );
        const lastUpdate = element(
            `last-update-${sensor}`
        );

        temperature.classList.toggle("disconnected", disconnected);
        lastUpdate.classList.toggle("disconnected", disconnected);

        temperature.textContent =
            disabled
                ? `-- °${unit}`
                : disconnected
                    ? "Error"
                    : point
                ? `${displayTemperature(point.y).toFixed(1)} °${unit}`
                : `-- °${unit}`;

        lastUpdate.textContent =
            disabled
                ? "Sensor off"
                : disconnected
                    ? "Device disconnected"
                    : point
                ? `Last update: ${formatTime(point.x)}` +
                  `${point.x < cutoff ? " (stale)" : ""}`
                : "Waiting for data...";
    }
}


/* -------------------------------------------------------------------------- */
/* Temperature history                                                        */
/* -------------------------------------------------------------------------- */

async function loadTemperatureHistory() {
    const since =
        new Date(
            Date.now() -
            WINDOW_MS
        ).toISOString();

    const until =
        new Date().toISOString();

    const pageSize = 500;

    // This runs only once when the page first opens.
    for (
        let offset = 0;
        ;
        offset += pageSize
    ) {
        const {
            data,
            error
        } = await client
            .from("temperature_readings")
            .select(
                "id,sensor_id,temperature,recorded_at"
            )
            .in(
                "sensor_id",
                SENSOR_IDS
            )
            .gte(
                "recorded_at",
                since
            )
            .lte(
                "recorded_at",
                until
            )
            .order("recorded_at")
            .order("id")
            .range(
                offset,
                offset + pageSize - 1
            );

        if (error) {
            throw error;
        }

        addReadings(data);

        if (data.length < pageSize) {
            break;
        }
    }
}

async function pollNewTemperatures() {
    // Prevent requests from overlapping.
    if (temperaturePollInProgress) {
        return;
    }

    temperaturePollInProgress = true;

    try {
        let newestReadingTime = 0;

        for (
            const point
            of latest.values()
        ) {
            newestReadingTime =
                Math.max(
                    newestReadingTime,
                    point.x
                );
        }

        /*
         * Ask only for rows near or after the
         * newest reading already displayed.
         *
         * The two-second overlap prevents readings
         * with identical timestamps from being missed.
         * addReadings() removes any duplicates.
         */
        const since =
            new Date(
                newestReadingTime > 0
                    ? newestReadingTime -
                      TEMPERATURE_POLL_OVERLAP_MS
                    : Date.now() -
                      WINDOW_MS
            ).toISOString();

        const {
            data,
            error
        } = await client
            .from("temperature_readings")
            .select(
                "id,sensor_id,temperature,recorded_at"
            )
            .in(
                "sensor_id",
                SENSOR_IDS
            )
            .gte(
                "recorded_at",
                since
            )
            .order("recorded_at")
            .order("id")
            .limit(
                TEMPERATURE_POLL_BATCH_SIZE
            );

        if (error) {
            throw error;
        }

        addReadings(data);

        element(
            "connection-status"
        ).textContent =
            realtimeConnected
                ? "🟢 Live"
                : "🟡 Live via polling";
    } catch (error) {
        console.error(
            "Could not poll new temperatures:",
            error
        );

        if (!realtimeConnected) {
            element(
                "connection-status"
            ).textContent =
                "🔴 Poll failed — retrying";
        }
    } finally {
        temperaturePollInProgress = false;
    }
}


/* -------------------------------------------------------------------------- */
/* Sensor state                                                               */
/* -------------------------------------------------------------------------- */

function updateSensorStatus(row) {
    const sensor =
        Number(row.sensor_id);

    if (
        !SENSOR_IDS.includes(sensor)
    ) {
        return;
    }

    if (row.enabled === true && sensorEnabled.get(sensor) === false) {
        waitingSince.set(sensor, Date.now());
        latest.delete(sensor);
    }
    sensorEnabled.set(sensor, row.enabled);

    element(
        `sensor-${sensor}-status`
    ).textContent =
        row.enabled == null
            ? "Unknown"
            : row.enabled
                ? "🟢 ON"
                : "🔴 OFF";

    render();
}

async function loadSensorState() {
    const {
        data,
        error
    } = await client
        .from("sensor_state")
        .select(
            "sensor_id,enabled"
        )
        .in(
            "sensor_id",
            SENSOR_IDS
        );

    if (error) {
        console.error(
            "Could not load sensor state:",
            error
        );

        return;
    }

    data.forEach(
        updateSensorStatus
    );
}

async function setSensor(
    sensor,
    enabled
) {
    sensor = Number(sensor);

    if (
        !SENSOR_IDS.includes(sensor)
    ) {
        return;
    }

    const status =
        element("command-status");

    status.textContent =
        `Setting Sensor ${sensor} ` +
        `${enabled ? "ON" : "OFF"}...`;

    const {
        error
    } = await client
        .from("sensor_state")
        .update({
            enabled
        })
        .eq(
            "sensor_id",
            sensor
        );

    if (error) {
        status.textContent =
            `❌ Could not update Sensor ${sensor}`;

        console.error(
            "Sensor state update failed:",
            error
        );

        return;
    }

    status.textContent =
        `✓ Sensor ${sensor} set to ` +
        `${enabled ? "ON" : "OFF"}; ` +
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
    const {
        data,
        error
    } = await client
        .from("alert_settings")
        .select(
            "sensor_id,max_temp,min_temp," +
            "contact_email,high_message,low_message"
        )
        .in(
            "sensor_id",
            SENSOR_IDS
        );

    if (error) {
        console.error(
            "Could not load alert settings:",
            error
        );

        return;
    }

    for (const row of data) {
        const sensor =
            Number(row.sensor_id);

        element(
            `max-temp-${sensor}`
        ).value =
            row.max_temp ?? "";

        element(
            `min-temp-${sensor}`
        ).value =
            row.min_temp ?? "";

        element(
            `contact-email-${sensor}`
        ).value =
            row.contact_email ?? "";

        element(
            `high-message-${sensor}`
        ).value =
            row.high_message ?? "";

        element(
            `low-message-${sensor}`
        ).value =
            row.low_message ?? "";

        alertThresholds.set(
            sensor,
            {
                max:
                    row.max_temp == null
                        ? NaN
                        : Number(
                            row.max_temp
                        ),

                min:
                    row.min_temp == null
                        ? NaN
                        : Number(
                            row.min_temp
                        )
            }
        );
    }

    render();
}

async function saveAlertSettings(
    sensor
) {
    sensor = Number(sensor);

    if (
        !SENSOR_IDS.includes(sensor)
    ) {
        return;
    }

    const status =
        element(
            `alert-status-${sensor}`
        );

    const maxTemp =
        Number(
            element(
                `max-temp-${sensor}`
            ).value
        );

    const minTemp =
        Number(
            element(
                `min-temp-${sensor}`
            ).value
        );

    status.textContent =
        "Saving...";

    if (
        !Number.isFinite(maxTemp) ||
        !Number.isFinite(minTemp) ||
        maxTemp <= minTemp
    ) {
        status.textContent =
            "❌ Max temperature must be greater " +
            "than min temperature";

        return;
    }

    const {
        error
    } = await client
        .from("alert_settings")
        .update({
            max_temp: maxTemp,
            min_temp: minTemp,

            contact_email:
                element(
                    `contact-email-${sensor}`
                ).value,

            high_message:
                element(
                    `high-message-${sensor}`
                ).value,

            low_message:
                element(
                    `low-message-${sensor}`
                ).value
        })
        .eq(
            "sensor_id",
            sensor
        );

    if (error) {
        status.textContent =
            "❌ Could not save alert settings";

        console.error(
            "Alert settings save failed:",
            error
        );

        return;
    }

    status.textContent =
        "✓ Alert settings saved";

    alertThresholds.set(
        sensor,
        {
            max: maxTemp,
            min: minTemp
        }
    );

    render();
}


/* -------------------------------------------------------------------------- */
/* Refresh / connection handling                                              */
/* -------------------------------------------------------------------------- */

async function refresh() {
    if (refreshing) {
        return;
    }

    refreshing = true;

    try {
        await Promise.all([
            pollNewTemperatures(),
            loadSensorState()
        ]);
    } finally {
        refreshing = false;
    }
}


/* -------------------------------------------------------------------------- */
/* Supabase Realtime                                                          */
/* -------------------------------------------------------------------------- */

client
    .channel(
        "temperature-monitor"
    )

    .on(
        "postgres_changes",
        {
            event: "INSERT",
            schema: "public",
            table:
                "temperature_readings"
        },

        ({ new: row }) =>
            addReadings([row])
    )

    .on(
        "postgres_changes",
        {
            event: "*",
            schema: "public",
            table: "sensor_state"
        },

        ({ new: row }) =>
            updateSensorStatus(row)
    )

    .subscribe(
        (status) => {
            realtimeConnected =
                status === "SUBSCRIBED";

            element(
                "connection-status"
            ).textContent =
                realtimeConnected
                    ? "🟢 Live"
                    : "🟡 Connecting — polling enabled";

            if (realtimeConnected) {
                refresh();
            }
        }
    );


/* -------------------------------------------------------------------------- */
/* Timers and startup                                                         */
/* -------------------------------------------------------------------------- */

// Advance the time window even if the sensors stop sending data.
setInterval(
    render,
    250
);

// Check for new temperature readings every 500 ms.
setInterval(
    pollNewTemperatures,
    TEMPERATURE_POLL_INTERVAL_MS
);

// Sensor state does not need to be checked as frequently.
setInterval(
    loadSensorState,
    5000
);

// Immediately catch up when returning to the browser tab.
document.addEventListener(
    "visibilitychange",
    () => {
        if (!document.hidden) {
            render();
            pollNewTemperatures();
            loadSensorState();
        }
    }
);

// Load the complete 300-second graph once at startup.
loadTemperatureHistory()
    .then(
        pollNewTemperatures
    )
    .catch(
        (error) => {
            console.error(
                "Could not load temperature history:",
                error
            );

            element(
                "connection-status"
            ).textContent =
                "🔴 Initial load failed — retrying";
        }
    );

loadSensorState();
loadAlertSettings();
