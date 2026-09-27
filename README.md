# 🌐 Firebase IoT Environment Monitor

### Task 5 — Firebase Logging, Automation & Data Export

> **A complete end-to-end IoT environment monitoring and automated bulb-control system using ESP32, DHT11, LDR, Firebase Realtime Database and a web dashboard.**

---

## 📌 Overview

This project extends the Firebase dashboard developed in **Task 4** into a complete working IoT system.

The system:

* 🌡️ Monitors **temperature**
* 💧 Monitors **humidity**
* 💡 Measures **ambient light using an LDR**
* 🔐 Uses Firebase Authentication
* ☁️ Stores live data in Firebase Realtime Database
* 🔄 Supports **Manual and Automatic modes**
* 🤖 Automatically controls the bulb using temperature and humidity thresholds
* 📝 Logs sensor readings with timestamps
* 📊 Displays historical data
* 📈 Displays live environment trends
* 📥 Exports loaded historical data as CSV
* 🚀 Runs the dashboard through Firebase Hosting

The project builds directly on the Firebase cloud infrastructure created in **Task 4**.

---

# 🎯 Objectives

The main objectives of Task 5 are:

1. Connect physical sensors to an ESP32.
2. Read temperature and humidity using a DHT11.
3. Read ambient light using an LDR.
4. Send live sensor readings to Firebase.
5. Implement Manual bulb control.
6. Implement Automatic bulb control.
7. Use temperature and humidity thresholds for automation.
8. Implement hysteresis to prevent rapid bulb switching.
9. Log readings with Firebase server timestamps.
10. Display historical records on the dashboard.
11. Display live sensor trends using a chart.
12. Export historical data to CSV.
13. Demonstrate the complete ESP32 → Firebase → Dashboard → ESP32 communication path.

---

# 🧠 Concepts Covered

## 🔄 Manual and Automatic Mode

### Manual Mode

In Manual mode, the dashboard directly determines the bulb state.

```text
Dashboard
    ↓
Firebase control/bulb
    ↓
ESP32
    ↓
Relay
    ↓
Bulb
```

### Automatic Mode

In Auto mode, the ESP32 makes the decision based on temperature and humidity.

```text
DHT11
  ↓
Temperature + Humidity
  ↓
ESP32 Automation Logic
  ↓
Threshold + Hysteresis
  ↓
Relay
  ↓
Bulb
```

---

# 🌡️ Temperature + Humidity Automation

The automatic mode uses both temperature and humidity.

The bulb turns **ON** when:

```text
Temperature >= 30°C
OR
Humidity >= 70%
```

The bulb turns **OFF** only when:

```text
Temperature <= 26°C
AND
Humidity < 70%
```

This creates a hysteresis band.

---

# 🔁 Why Hysteresis?

Without hysteresis, a sensor reading close to a threshold could repeatedly switch the bulb:

```text
30.0°C → ON
29.9°C → OFF
30.0°C → ON
29.9°C → OFF
```

This can cause relay chatter.

Instead, the project uses separate ON and OFF thresholds:

```text
             BULB ON
               ▲
               │
Temperature ≥ 30°C
OR Humidity ≥ 70%
               │
        ───────┼───────
               │
          Dead Band
               │
        ───────┼───────
               │
Temperature ≤ 26°C
AND Humidity < 70%
               │
               ▼
             BULB OFF
```

The last automatic state is stored in:

```cpp
autoBulbState
```

so the bulb remains stable inside the dead-band.

---

# 💡 LDR Function

The LDR continuously measures ambient light.

Its raw analog value is read from:

```text
GPIO 34
```

The value is:

* Sent to Firebase
* Displayed on the dashboard
* Stored in historical logs

The current implementation **does not use the LDR as an automatic trigger**.

The constant:

```cpp
#define LDR_DARK_THRESHOLD 1500
```

is retained as a reference for possible future light-based automation.

---

# 🏗️ System Architecture

```text
                 ┌───────────────┐
                 │     DHT11     │
                 │ Temp/Humidity │
                 └───────┬───────┘
                         │
                         ▼
                 ┌───────────────┐
                 │     LDR       │
                 │ Ambient Light │
                 └───────┬───────┘
                         │
                         ▼
                  ┌─────────────┐
                  │    ESP32    │
                  │             │
                  │ Read Sensors│
                  │ Automation  │
                  │ Hysteresis  │
                  └──────┬──────┘
                         │
                    Wi-Fi│
                         ▼
             ┌─────────────────────┐
             │ Firebase Realtime   │
             │ Database            │
             │                     │
             │ sensors             │
             │ control             │
             │ logs                │
             │ status              │
             └──────────┬──────────┘
                        │
                        ▼
              ┌──────────────────┐
              │ Web Dashboard    │
              │                  │
              │ Live Readings    │
              │ History          │
              │ Chart            │
              │ Manual Control   │
              │ CSV Export       │
              └────────┬─────────┘
                       │
                 control commands
                       │
                       ▼
                     ESP32
                       │
                       ▼
                     Relay
                       │
                       ▼
                     Bulb
```

---

# 🔧 Hardware

| Component                   | Purpose                              |
| --------------------------- | ------------------------------------ |
| **ESP32 Development Board** | Main IoT controller                  |
| **DHT11**                   | Temperature and humidity measurement |
| **LDR**                     | Ambient light measurement            |
| **Fixed Resistor**          | LDR voltage divider                  |
| **2-Channel Relay Module**  | Electrical switching                 |
| **Bulb**                    | Controlled load                      |
| **Breadboard**              | Circuit assembly                     |
| **Jumper Wires**            | Electrical connections               |

---

# 🔌 Wiring

| ESP32 Pin | Connected Component | Purpose                |
| --------- | ------------------- | ---------------------- |
| `GPIO 4`  | DHT11 Data          | Temperature + humidity |
| `GPIO 34` | LDR voltage divider | Ambient light          |
| `GPIO 5`  | Relay IN            | Bulb control           |

### DHT11

```text
DHT11 DATA
    │
    └──────── GPIO 4
```

### LDR

```text
3.3V
 │
[LDR]
 │
 ├──────── GPIO 34
 │
[Fixed Resistor]
 │
GND
```

### Relay

```text
ESP32 GPIO 5
      │
      ▼
 Relay IN
      │
      ▼
 Relay Contact
      │
      ▼
    Bulb
```

> ⚠️ The bulb operates at mains voltage. The mains side must remain properly isolated from the ESP32 low-voltage circuitry. Do not connect mains voltage directly to an ESP32 GPIO.

---

# ⚙️ Automation Configuration

| Parameter                 |      Value | Function                  |
| ------------------------- | ---------: | ------------------------- |
| `TEMP_HIGH_THRESHOLD`     |   `30.0°C` | Turns bulb ON             |
| `TEMP_LOW_THRESHOLD`      |   `26.0°C` | Allows bulb OFF           |
| `HUMIDITY_HIGH_THRESHOLD` |      `70%` | Turns bulb ON             |
| `LDR_DARK_THRESHOLD`      |     `1500` | Reference only            |
| `READ_INTERVAL_MS`        |  `3000 ms` | Sensor reading interval   |
| `LOG_INTERVAL_MS`         | `15000 ms` | Database logging interval |

---

# ☁️ Firebase Database Structure

The same Firebase project and structure from Task 4 are used.

```text
Firebase Realtime Database
│
├── sensors
│   ├── temperature
│   ├── humidity
│   └── light
│
├── control
│   ├── mode
│   └── bulb
│
├── logs
│   └── timestamped records
│
└── status
    └── online
```

---

# 📡 Live Sensor Data

The ESP32 writes sensor readings to:

```text
sensors/temperature
sensors/humidity
sensors/light
```

Example:

```json
{
  "temperature": 27.1,
  "humidity": 63,
  "light": 1
}
```

The dashboard listens to these values and updates the displayed cards in real time.

---

# 🎛️ Control Data

The dashboard communicates with the ESP32 through:

```text
control
```

Example:

```json
{
  "mode": "manual",
  "bulb": true
}
```

or:

```json
{
  "mode": "auto",
  "bulb": false
}
```

---

# 📝 Historical Logging

Every:

```text
15 seconds
```

the ESP32 pushes a new record under:

```text
logs
```

Each log contains:

```text
timestamp
temperature
humidity
light
bulb
mode
```

Example:

```json
{
  "temperature": 27.1,
  "humidity": 63,
  "light": 1,
  "bulb": false,
  "mode": "auto",
  "timestamp": "Firebase server timestamp"
}
```

Each reading is stored as a new child instead of overwriting the previous reading.

This creates a historical record of system operation.

---

# ⏱️ Firebase Server Timestamp

The ESP32 uses:

```cpp
json.set("timestamp/.sv", "timestamp");
```

This requests a Firebase server-side timestamp.

This is preferable to relying on the ESP32's local clock because the database provides the timestamp consistently.

---

# 📊 Dashboard

The dashboard provides:

### Live Environment Cards

* Temperature
* Humidity
* Ambient light
* Device status

### Controls

* Manual/AUTO mode
* Bulb ON/OFF

### Historical Data

Displays logged readings from:

```text
logs
```

### Live Environment Trends

A chart visualizes sensor data over time.

### CSV Export

The currently loaded historical rows can be downloaded as:

```text
.csv
```

---

# 🔐 Authentication

The ESP32 uses a dedicated Firebase Authentication account.

Example:

```text
device@example.com
```

This is separate from the human dashboard login.

### Human

```text
User
 ↓
Firebase Authentication
 ↓
Dashboard
```

### ESP32

```text
ESP32
 ↓
Dedicated Firebase Auth account
 ↓
Realtime Database
```

This allows both the dashboard and ESP32 to satisfy the database authentication requirements.

---

# 🔒 Firebase Security

The project continues to use authenticated Firebase Realtime Database access from Task 4.

The major nodes are:

```text
sensors
control
logs
status
```

Access is gated using:

```text
auth != null
```

The `logs` node also uses:

```json
".indexOn": ["timestamp"]
```

to support timestamp-based queries efficiently.

---

# 💻 Software

| Software                       | Version / Details          |
| ------------------------------ | -------------------------- |
| **Arduino IDE**                | 2.3.10                     |
| **Firebase Arduino Client**    | Mobizt v4.4.17             |
| **DHT Sensor Library**         | Adafruit                   |
| **Adafruit Unified Sensor**    | DHT dependency             |
| **Firebase Web SDK**           | Dashboard integration      |
| **Firebase Hosting**           | Dashboard deployment       |
| **Firebase Realtime Database** | Cloud storage              |
| **Firebase Authentication**    | User/device authentication |

---

# 📚 Required Arduino Libraries

Install the following using:

```text
Arduino IDE
→ Sketch
→ Include Library
→ Manage Libraries
```

Search for:

```text
Firebase ESP32 Client
```

by:

```text
Mobizt
```

Also install:

```text
DHT sensor library
```

by:

```text
Adafruit
```

The DHT library also requires:

```text
Adafruit Unified Sensor
```

---

# 🧑‍💻 ESP32 Firmware

The main firmware file is:

```text
task5_esp32_firebase.ino
```

The firmware performs five major functions:

```text
1. Connect ESP32 to Wi-Fi
2. Authenticate with Firebase
3. Read DHT11 + LDR
4. Execute Manual/Auto bulb logic
5. Upload sensor readings and logs
```

---

# 🔄 Firmware Workflow

```text
ESP32 starts
     │
     ▼
Connect Wi-Fi
     │
     ▼
Authenticate with Firebase
     │
     ▼
Set device online
     │
     ▼
Read DHT11 + LDR
     │
     ▼
Upload live sensors
     │
     ▼
Read control/mode
     │
     ├───────────────┐
     │               │
   MANUAL           AUTO
     │               │
     ▼               ▼
Read bulb       Check thresholds
from Firebase       │
     │               ▼
     │           Hysteresis
     │               │
     └───────┬───────┘
             ▼
          Relay
             │
             ▼
           Bulb
             │
             ▼
       Create log
       every 15s
```

---

# 🧪 Testing

## Test 1 — ESP32 Compilation

The firmware was compiled for:

```text
ESP32 Dev Module
```

---

## Test 2 — ESP32 Upload

The firmware was successfully uploaded to the ESP32.

Arduino IDE displayed:

```text
Done uploading
```

---

## Test 3 — Live Sensor Monitoring

The dashboard displayed real sensor values.

Example:

```text
Temperature: 27.1°C
Humidity:    63%
Light:       1
Status:      Device online
```

---

## Test 4 — Manual Mode

The dashboard was switched to Manual mode.

The bulb state followed:

```text
control/bulb
```

---

## Test 5 — Automatic Mode

The dashboard was switched to Auto mode.

The ESP32 then controlled the bulb based on:

```text
Temperature
+
Humidity
```

---

## Test 6 — Historical Logging

Firebase was inspected to confirm that records contained:

```text
bulb
humidity
light
mode
temperature
timestamp
```

---

## Test 7 — CSV Export

The dashboard's historical records were downloaded as CSV and verified against the records stored in Firebase.

---

# 📸 Evidence

The project contains evidence for:

### Hardware

* ESP32
* Relay module
* Bulb
* Breadboard
* Sensor wiring

### Software

* Firebase library installation
* Arduino IDE configuration
* Compilation
* ESP32 upload

### Firebase

* Authentication
* Realtime Database
* Historical logs

### Dashboard

* Login
* Live readings
* Device online status
* Live chart
* Historical data
* Manual/Auto operation

### Physical Operation

* Bulb OFF
* Bulb ON

---

# 🛠️ Challenges & Fixes

## 1. Changing Automation from LDR to Temperature + Humidity

The original concept considered LDR-based automatic lighting.

The final implementation uses:

```text
Temperature + Humidity
```

as the active automatic trigger.

The LDR remains available for monitoring and future automation.

---

## 2. Preventing Relay Flicker

A single threshold could cause rapid ON/OFF switching.

The solution was hysteresis:

```text
ON:
Temperature ≥ 30°C
OR
Humidity ≥ 70%

OFF:
Temperature ≤ 26°C
AND
Humidity < 70%
```

The previous automatic state is stored in:

```cpp
autoBulbState
```

---

## 3. Maintaining Historical Records

Instead of overwriting a single database value, the ESP32 uses:

```cpp
Firebase.RTDB.pushJSON()
```

to create a new record for every logging cycle.

This produces a historical dataset.

---

## 4. Verifying Auto Mode

Historical records can contain the same temperature while showing different bulb states.

This is possible because hysteresis depends on the previous automatic state and the other threshold condition, not only the current temperature value.

---

# 📈 Data Flow

### Sensor → Firebase

```text
DHT11
 ↓
ESP32
 ↓
Firebase
 ↓
sensors
```

### Firebase → Dashboard

```text
Firebase
 ↓
Firebase Web SDK
 ↓
Dashboard
 ↓
Temperature / Humidity / Light
```

### Dashboard → Firebase

```text
Dashboard
 ↓
Firebase
 ↓
control
```

### Firebase → ESP32

```text
control
 ↓
ESP32
 ↓
Relay
 ↓
Bulb
```

### ESP32 → Historical Logs

```text
ESP32
 ↓
Firebase
 ↓
logs
 ↓
Dashboard
 ↓
Historical Table
 ↓
CSV
```

---

# 🔮 Future Improvements

Possible future improvements include:

### 💡 Light-Based Automation

Use the existing:

```cpp
LDR_DARK_THRESHOLD
```

to include ambient light in automatic decisions.

For example:

```text
Dark
+
Temperature/Humidity condition
        ↓
      Bulb ON
```

---

### 📊 Full Historical Export

The current CSV export operates on the rows currently loaded by the dashboard.

A future version could retrieve all available `logs` records before generating the CSV.

---

### 📱 Mobile Interface

The dashboard could be further optimized for mobile devices.

---

### 🔔 Alerts

Firebase Cloud Functions or another notification service could be used to generate alerts when:

```text
Temperature is too high
Humidity is too high
Device goes offline
```

---

# 💭 Reflection

This task extends the cloud infrastructure created in Task 4 into a complete IoT system.

Task 4 established:

```text
Authentication
+
Realtime Database
+
Dashboard
+
Firebase Hosting
```

Task 5 adds:

```text
ESP32
+
DHT11
+
LDR
+
Relay
+
Automation
+
Historical Logging
+
CSV Export
```

The result is a complete two-way IoT communication system.

The ESP32 does not simply send sensor values to the cloud. It can also make an autonomous control decision based on temperature and humidity.

At the same time, the dashboard can still override the system through Manual mode.

The project therefore demonstrates both:

```text
Human-controlled operation
```

and:

```text
Autonomous operation
```

while maintaining a timestamped history of the system's activity.

---

# 🔗 Project Links

### 🌐 Live Login

https://env-monitor-845af.web.app/index.html

### 📊 Live Dashboard

https://env-monitor-845af.web.app/dashboard.html

### 💻 GitHub Repository

https://github.com/sachinsivapragash/Firebase-Logging-Automation-Data-Export.git

### 📁 Source Files & Historical Data

https://drive.google.com/drive/folders/1xW0GiJWKqV8Wh8Vf9mvSClvzJ3pUDE5f?usp=drive_link

---

# 👤 Author

**Sachin S**

### Project

**Firebase IoT Environment Monitoring, Automation & Data Export**

### Technologies

`ESP32` · `DHT11` · `LDR` · `Firebase` · `Realtime Database` · `Authentication` · `Arduino` · `HTML` · `CSS` · `JavaScript`

### Deployment

**Firebase Hosting**

---

# 🏁 Conclusion

The **Firebase IoT Environment Monitor** successfully demonstrates an end-to-end IoT system in which physical sensor data is collected by an ESP32, transmitted to Firebase, displayed through a web dashboard, stored as historical records, and used for automated bulb control.

The final system combines:

**ESP32 → Sensors → Firebase → Dashboard → Control → Relay → Bulb**

with:

**Authentication + Automation + Hysteresis + Historical Logging + CSV Export**

making the project a complete cloud-connected IoT monitoring and control implementation.
