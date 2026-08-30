// ============================================
// SUPABASE CONFIGURATION
// ============================================
const SUPABASE_URL =
    "https://witvuhdpcpfolevfjapq.supabase.co";

const SUPABASE_KEY =
    "sb_publishable_sbNdNAgtIPJu5M8eq8u4uQ_yZfwFBnN";

const DEVICE_ID = 1;

const supabaseClient = supabase.createClient(
    SUPABASE_URL,
    SUPABASE_KEY
);

// ============================================
// DOM ELEMENTS
// ============================================

const currentTemperature =
    document.getElementById(
        "current-temperature"
    );

const lastUpdate =
    document.getElementById(
        "last-update"
    );

const connectionStatus =
    document.getElementById(
        "connection-status"
    );

const sensor1Status =
    document.getElementById(
        "sensor-1-status"
    );

const sensor2Status =
    document.getElementById(
        "sensor-2-status"
    );

const commandStatus =
    document.getElementById(
        "command-status"
    );


// ============================================
// TEMPERATURE CHART
// ============================================

const chart = new Chart(
    document.getElementById(
        "temperature-chart"
    ),
    {
        type: "line",

        data: {
            labels: [],

            datasets: [
                {
                    label: "Temperature",

                    data: [],

                    borderWidth: 2,

                    pointRadius: 0,

                    tension: 0.2
                }
            ]
        },

        options: {

            responsive: true,

            maintainAspectRatio: false,

            animation: false,

            scales: {

                x: {
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
    }
);


// ============================================
// ADD TEMPERATURE TO GRAPH
// ============================================

function addTemperatureReading(
    reading
) {

    const time =
        new Date(
            reading.created_at
        );


    const label =
        time.toLocaleTimeString(
            [],
            {
                hour: "2-digit",

                minute: "2-digit",

                second: "2-digit"
            }
        );


    chart.data.labels.push(
        label
    );


    chart.data.datasets[0].data.push(
        reading.temperature
    );


    // Keep only the last 300 readings

    while (
        chart.data.labels.length > 300
    ) {

        chart.data.labels.shift();

        chart.data.datasets[0]
            .data.shift();
    }


    chart.update("none");
}


// ============================================
// LOAD TEMPERATURE HISTORY
// ============================================

async function loadTemperatureHistory()
{
    const {
        data,
        error
    } =
        await supabaseClient

            .from(
                "temperature_readings"
            )

            .select("*")

            .eq(
                "device_id",
                DEVICE_ID
            )

            .order(
                "created_at",
                {
                    ascending: true
                }
            );


    console.log(
        "Temperature data:",
        data
    );

    console.log(
        "Temperature error:",
        error
    );


    if (error) {

        console.error(
            "Could not load history:",
            error
        );

        return;
    }


    console.log(
        `Received ${data.length} rows`
    );


    for (
        const reading of data
    ) {

        addTemperatureReading(
            reading
        );
    }


    if (data.length > 0) {

        const latest =
            data[data.length - 1];

        updateCurrentTemperature(
            latest
        );
    }
}


// ============================================
// UPDATE CURRENT TEMPERATURE
// ============================================

function updateCurrentTemperature(
    reading
) {

    currentTemperature.textContent =
        `${reading.temperature.toFixed(1)} °F`;


    const date =
        new Date(
            reading.created_at
        );


    lastUpdate.textContent =
        `Last update: ${
            date.toLocaleTimeString()
        }`;
}


// ============================================
// REALTIME TEMPERATURE UPDATES
// ============================================

function subscribeToTemperature()
{
    supabaseClient

        .channel(
            "temperature_readings"
        )

        .on(
            "postgres_changes",
            {
                event: "INSERT",

                schema: "public",

                table: "temperature_readings",

                filter:
                    `device_id=eq.${DEVICE_ID}`
            },

            (payload) => {

                console.log(
                    "New temperature:",
                    payload.new
                );


                addTemperatureReading(
                    payload.new
                );


                updateCurrentTemperature(
                    payload.new
                );
            }
        )

        .subscribe(
            (status) => {

                console.log(
                    "Realtime status:",
                    status
                );


                if (
                    status === "SUBSCRIBED"
                ) {

                    connectionStatus.textContent =
                        "🟢 Connected";

                }

                else {

                    connectionStatus.textContent =
                        "🟡 Connecting...";
                }
            }
        );
}


// ============================================
// GET CURRENT DEVICE STATE
// ============================================

async function loadDeviceState()
{
    const {
        data,
        error
    } =
        await supabaseClient

            .from("device_state")

            .select(
                "temperature_1_enabled, temperature_2_enabled"
            )

            .eq(
                "device_id",
                DEVICE_ID
            )

            .single();


    if (error) {

        console.error(
            "Could not load device state:",
            error
        );

        return;
    }


    updateSensorStatus(
        1,
        data.temperature_1_enabled
    );

    updateSensorStatus(
        2,
        data.temperature_2_enabled
    );
}


// ============================================
// DISPLAY SENSOR STATE
// ============================================

function updateSensorStatus(
    sensorNumber,
    enabled
) {

    const statusElement =
        sensorNumber === 1
            ? sensor1Status
            : sensor2Status;


    if (enabled) {

        statusElement.textContent =
            "🟢 ON";

    } else {

        statusElement.textContent =
            "🔴 OFF";
    }
}


// ============================================
// SEND SENSOR COMMAND
// ============================================

async function setSensor(
    sensorNumber,
    enabled
) {

    commandStatus.textContent =
        `Sending Sensor ${sensorNumber} command...`;


    const command =
        `temperature_sensor_${sensorNumber}`;


    const {
        error
    } =
        await supabaseClient

            .from("device_commands")

            .insert({
                device_id:
                    DEVICE_ID,

                command:
                    command,

                value:
                    enabled
            });


    if (error) {

        console.error(
            "Command failed:",
            error
        );


        commandStatus.textContent =
            `❌ Sensor ${sensorNumber} command failed`;

        return;
    }


    // Immediately update the UI

    updateSensorStatus(
        sensorNumber,
        enabled
    );


    commandStatus.textContent =
        enabled
            ? `✓ Sensor ${sensorNumber} ON command sent`
            : `✓ Sensor ${sensorNumber} OFF command sent`;
}


// ============================================
// START APPLICATION
// ============================================

async function start()
{
    await loadTemperatureHistory();

    await loadDeviceState();

    subscribeToTemperature();
}

start();