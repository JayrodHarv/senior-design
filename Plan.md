# Project Plan

> **Goal:** Build and connect the embedded temperature-monitoring device, web interface, cloud backend, and notification system.

> **Development Note:** Each step should be developed and tested independently before connecting the pieces together.

> **Tools:** We will use the **PlatformIO extension in VS Code** and write most of the embedded code in **C++**.

---

## 1. 🔧 Build & Test the Third Box

**Priority:** Get everything working before worrying about appearance.

### Hardware

* Connect the microcontroller
* Connect temperature sensors
* Connect display
* Connect buttons/toggle switches
* Connect battery
* Connect power switch
* Connect any required connectors

### Functionality

* [ ] Microcontroller powers on and runs reliably
* [ ] Temperature sensors provide accurate readings
* [ ] Temperature data is displayed on the screen
* [ ] Toggle buttons function correctly
* [ ] Power switch functions correctly
* [ ] Store **300 seconds of temperature data** in memory
* [ ] Store a **timestamp with each temperature reading**

> **Milestone:** The third box should be completely functional, even if it looks ugly.

---

## 2. 🌐 Build the Web Interface

Create the website using **dummy data** first.

### Technologies

* HTML
* CSS
* JavaScript
* [Bootstrap](https://getbootstrap.com/) for styling

### Functionality

* [ ] Create basic webpage layout
* [ ] Create temperature graph
* [ ] Populate graph with dummy temperature data
* [ ] Make the interface readable and usable
* [ ] Test the interface independently from the ESP32

> **Milestone:** A functional web interface that can display temperature data without needing the physical device.

---

## 3. ☁️ Connect the ESP32 to the Cloud

Use a cloud service to store and retrieve temperature data.

### Proposed Service

[Supabase](https://supabase.com/) — has a free tier and provides a database + API.

### ESP32 → Cloud

* [ ] Create Supabase project
* [ ] Create database table for temperature readings
* [ ] Configure API access
* [ ] Have the ESP32 send temperature data to the API
* [ ] Verify data is correctly stored in the database

### Cloud → Web Interface

* [ ] Have the web interface request temperature data
* [ ] Retrieve data from the API
* [ ] Display real temperature data on the graph

> **Milestone:**
> **ESP32 → API → Database → Web Interface**

---

## 4. 📧 Add Temperature Notifications

Trigger a notification when a temperature threshold is exceeded.

### Approach

We'll use **email notifications** rather than text messages because they should be simpler to implement.

The notification logic will run in the **cloud**, rather than on the ESP32.

### Tasks

* [ ] Define high/low temperature thresholds
* [ ] Detect threshold violations in the cloud
* [ ] Implement email notification system
* [ ] Test notifications with simulated temperature data
* [ ] Test notifications using real ESP32 data

> **Potential advantage:** We can continue using **Supabase** for the cloud-side logic. Yippee.

---

## 5. 🧰 Design & Build the Enclosure

Once the electronics are fully tested, make the device look less like a collection of parts.

### Tasks

* [ ] Measure final component dimensions
* [ ] Design enclosure in CAD
* [ ] Account for:

  * Display
  * Buttons
  * Power switch
  * Temperature sensors
  * Battery
  * Connectors
  * Ventilation/sensor access
* [ ] 3D print enclosure
* [ ] Assemble and test final device

### Backup Plan

If the 3D printer betrays us:

> **Shove it in Tupperware.**

---

# 🔄 Overall System

Once all components have been developed independently, connect everything together:

```text
┌──────────────────┐
│   Temperature    │
│     Sensors      │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│      ESP32       │
│                  │
│ • Read sensors   │
│ • Store 300 sec  │
│ • Display data   │
└────────┬─────────┘
         │
         │ Internet
         ▼
┌──────────────────┐
│     Supabase     │
│                  │
│ • Database       │
│ • API            │
│ • Notifications  │
└───────┬──────────┘
        │
        ├───────────────┐
        │               │
        ▼               ▼
┌──────────────┐  ┌──────────────┐
│  Web Interface│  │    Email     │
│              │  │ Notifications │
│ • Graph      │  │              │
│ • Data       │  │ • High temp  │
│ • Monitoring │  │ • Low temp   │
└──────────────┘  └──────────────┘
```

---

## 🎯 Final Milestone

At the end of the project, the complete system should:

1. **Measure** temperature using the physical device
2. **Store** at least 300 seconds of local temperature history
3. **Send** temperature data to the cloud
4. **Store** data in a cloud database
5. **Display** temperature data on a webpage
6. **Detect** temperature threshold violations
7. **Send** email notifications when thresholds are exceeded
8. **Package** everything into a functional enclosure

### Development Philosophy

**Get it working → Get it connected → Get it pretty.**
