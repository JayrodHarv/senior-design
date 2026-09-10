# ECE 4880 Laboratory #1 — Thermometer with Web Interface

## Project Summary

The goal of this project is to **design and prototype a two-sensor thermometer system with both local and web-based controls and displays**.

The overall system consists of:

- A **computer** that provides the web/software user interface.
- **Two temperature sensors**, each attached to a durable cable.
- A battery-powered **“third box”** containing the electronics, local display, sensor buttons, connectors, battery, and main power switch.
- A **cellphone or email account** that can receive temperature alerts.

The third box must work as a standalone thermometer while also making temperature data available over the internet. The computer interface must show live temperature values, allow remote control of the sensor display states, display a continuously scrolling 300-second temperature history, handle missing/offline data correctly, and configure high/low temperature alerts.

The design is expected to be developed using a **rapid-prototyping approach**: get a basic working prototype early, then progressively add or revise features until every requirement is satisfied.

---

# Requirements Checklist

## 1. General System

- [x] The system includes a computer (desktop, laptop, or equivalent) used for the user interface, display, and control.
- [x] The system includes **two thermometer sensors**.
- [x] Each thermometer sensor is located at the end of a **1.0 ± 0.1 meter cable**.
- [ ] The sensor/cable assemblies are mechanically robust enough to withstand normal handling and bouncing around without breaking.
- [ ] The temperature sensors are not damaged when placed in ice water.
- [ ] The system includes a separate **third box** containing, at minimum:
  - [x] A local display.
  - [x] Buttons.
  - [ ] A battery.
  - [ ] A power switch.
- [ ] The third box and sensors can operate together as a **battery-powered thermometer**.
- [x] Temperature data from the thermometer is available over the internet.
- [x] The system includes a cellphone or email destination capable of receiving alert messages.

## 2. Mechanical Requirements for the Third Box

- [ ] The third box is enclosed.
- [ ] The third box is physically robust enough to survive being dropped from a workbench to the floor.
- [ ] The third box continues working when turned upside down.
- [ ] The internal circuit remains operational when the box is turned upside down.
- [ ] Connectors remain operational when the box is turned upside down.
- [ ] Switches/buttons remain operational when the box is turned upside down.
- [ ] All cable connections to the third box terminate in connectors that are **securely mounted to the box**.
- [ ] The connectors are designed to be easily connected and disconnected by a casual user.
- [ ] If the box is dropped with cables attached, the connectors do not break.
- [ ] If the box is dropped with cables attached, the cables do not break.
- [ ] It is acceptable for cables/connectors to become disconnected during a drop, as long as they are not damaged.
- [x] If a temperature sensor is unplugged, the third box continues normal operation without requiring user intervention.
- [x] If a temperature sensor is plugged back in, the third box resumes/maintains normal operation without requiring user intervention.

## 3. Main Power Switch

- [ ] The switch on the third box functions as the system **on/off switch**.
- [ ] When the switch is **off**, the thermometer system cannot display temperatures locally.
- [ ] When the switch is **off**, temperature data is not available from the internet.

## 4. Local Third-Box Operation

- [ ] When the main switch is **on**, local thermometer functionality is available.
- [x] Sensor 1 has its own button/control.
- [x] Sensor 2 has its own button/control.
- [x] When Sensor 1 is enabled, the local display shows Sensor 1's temperature in degrees Celsius.
- [x] When Sensor 2 is enabled, the local display shows Sensor 2's temperature in degrees Celsius.
- [ ] When Sensor 1 is disabled, the display shows **“Sensor 1 off”** or equivalent.
- [ ] When Sensor 2 is disabled, the display shows **“Sensor 2 off”** or equivalent.
- [ ] Pressing a sensor button causes the correct temperature/status to appear with **no noticeable delay**.
- [ ] Local button/display response time is approximately **20 ms or less** so that delay is not noticeable.
- [ ] The display is clearly readable under normal indoor lighting.
- [ ] The display correctly represents all temperatures within the required operating range.
- [ ] Both sensors can be enabled at the same time.
- [ ] Both sensors can be disabled at the same time.
- [ ] Sensor 1 can be enabled while Sensor 2 is disabled.
- [ ] Sensor 2 can be enabled while Sensor 1 is disabled.
- [ ] The display shows the correct information for all combinations of sensor on/off states.
- [ ] If Sensor 1 is unplugged or otherwise malfunctioning, the local display notifies the user of an error.
- [ ] If Sensor 2 is unplugged or otherwise malfunctioning, the local display notifies the user of an error.

## 5. Computer / Web Interface

### 5.1 Real-Time Temperature Display

- [x] When the third box is on and the computer is connected to the internet, the computer can display real-time data.
- [x] The computer displays the real-time temperature for **both sensors**.
- [x] The computer user can select between **degrees Celsius and degrees Fahrenheit**.
- [x] Real-time temperatures are displayed prominently in a **large font**.
- [x] Real-time temperature values update **once per second**.
- [x] If a sensor is unplugged, the real-time display shows an **“unplugged sensor”** message instead of a temperature.
- [x] If the third box is off, the real-time display shows **“no data available”** instead of a temperature.

### 5.2 Remote Sensor Display Control

- [x] The computer user can remotely turn Sensor 1's local third-box temperature display on or off.
- [x] The computer user can remotely turn Sensor 2's local third-box temperature display on or off.
- [x] The remote control acts like virtually pressing the corresponding button on the third box.
- [x] The third box responds to a remote sensor-control command in **less than 1 second**.

### 5.3 Temperature History Graph

- [x] The computer interface can display graph(s) of previous temperature readings.
- [x] The previous **300 seconds** of temperature data are available.
- [x] The 300-second graph becomes available within **10 seconds** of starting the computer software.
- [x] The graph can display temperature in degrees Celsius.
- [x] The graph can display temperature in degrees Fahrenheit.
- [x] The user can switch the graph between Celsius and Fahrenheit.
- [x] The graph's upper limit is always **50 °C / 122 °F**.
- [x] The graph's lower limit is always **10 °C / 50 °F**.
- [x] The graph scrolls horizontally.
- [x] The newest temperature value appears on the **right side** of the graph.
- [x] Older values appear toward the **left side** of the graph.
- [x] A new temperature value is added to the graph **once per second**.
- [x] Old values scroll off the left side of the graph.
- [x] The graph always represents a total history of **300 seconds**.
- [ ] The horizontal axis is labeled as **seconds ago from the current time**.
- [ ] The horizontal axis runs from approximately **300 seconds ago on the left to 0 seconds ago on the right**.
- [ ] Missing data is clearly visible on the graph.
- [ ] Missing data is visually distinguishable from data that is simply above or below the graph's scale.
- [x] If the third box is off, the graph continues scrolling and records the affected interval as missing data.
- [x] If a sensor is unplugged, the graph continues scrolling and records the affected interval as missing data.
- [x] When the third box or sensor becomes available again, graphing resumes automatically.
- [x] When the third box or sensor becomes available again, the real-time display resumes automatically.

## 6. Recovery When the Third Box Is Turned On

- [x] If the computer is already running while the third box is off, the graph appears/updates within **10 seconds** after the third box is turned on.
- [x] If the computer is already running while the third box is off, the real-time display appears/updates within **10 seconds** after the third box is turned on.

## 7. Temperature Alerts

- [x] When both the computer and third box are on, the system can send an alert when the real-time temperature exceeds a configurable maximum value.
- [x] The system can send an alert when the real-time temperature falls below a configurable minimum value.
- [x] Alerts can be sent by **text message and/or email** to a specified destination.
- [x] The user can change the **high-temperature alert message** from the computer interface.
- [x] The user can change the **low-temperature alert message** from the computer interface.
- [x] The user can change the **maximum temperature threshold** from the computer interface.
- [x] The user can change the **minimum temperature threshold** from the computer interface.
- [ ] The user can change the destination **phone number and/or email address** from the computer interface.

## 8. Temperature Operating Range and Verification

- [ ] The design supports displaying temperatures over at least **-10 °C to +63 °C**.
- [ ] The -10 °C to +63 °C operating range is addressed in the design even if the full range is not physically tested.
- [x] Holding a temperature sensor in a person's hand causes its measured temperature to rise after a few seconds.
- [ ] Holding a soldering iron close to, or briefly touching, a sensor causes its measured temperature to rise more quickly.
- [x] At normal laboratory room temperature, the thermometer reads approximately **22 °C ± 4 °C**.
- [ ] In a water-ice mixture, the thermometer reads approximately **0 °C ± 2 °C**.

---

## Final Project Goal

- [ ] The final design documentation clearly explains how to build the system.
- [x] The accompanying prototype demonstrates that the design works.
- [ ] All unclear or ambiguous assignment requirements have been clarified before the final design is completed.
- [x] A working prototype is produced early and progressively improved using the required rapid-prototyping approach.
- [ ] The final prototype and design satisfy all functional, performance, internet-interface, notification, and mechanical requirements listed above.
