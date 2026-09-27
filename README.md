
# Firebase Logging, Automation & Data Export

## 📌 Overview

A complete IoT monitoring and automation system using **ESP32, Firebase, DHT11, LDR, and a relay-controlled bulb**.

The system monitors temperature, humidity, and light, provides manual and automatic bulb control, stores timestamped readings in Firebase, and allows historical data to be viewed and downloaded as a CSV file.

This project builds on the Firebase monitoring dashboard by adding **sensor-based automation, historical logging, and data export**.

---

## 🎯 Objective

The objective is to build a complete cloud-connected IoT system that can:

- Monitor temperature, humidity, and light
- Control a bulb manually from the dashboard
- Control the bulb automatically using temperature and humidity
- Store sensor readings with timestamps
- Display historical data
- Export logged data as CSV
- Show device online/offline status

---

## 🧰 Hardware Used

- **ESP32 Development Board**
- **DHT11 Temperature & Humidity Sensor**
- **LDR**
- **Fixed Resistor**
- **2-Channel Relay Module**
- **Bulb**
- **Breadboard**
- **Jumper Wires**

---

## 💻 Software & Platforms

- **Arduino IDE 2.3.10**
- **Firebase Realtime Database**
- **Firebase Authentication**
- **Firebase Hosting**
- **Firebase Arduino Client Library**
- **DHT Sensor Library**
- **HTML**
- **CSS**
- **JavaScript**

---

## 🔌 Pin Configuration

| Component | ESP32 Pin |
|---|---|
| DHT11 Data | GPIO 4 |
| LDR | GPIO 34 |
| Relay / Bulb | GPIO 5 |

The LDR uses an analog voltage-divider configuration.

---

## 🏗️ System Design

```text
DHT11 ─────────┐
               │
LDR ───────────┤
               ↓
            ESP32
               │
       ┌───────┴────────┐
       ↓                ↓
 Firebase           Automation
       │                │
       ↓                ↓
 Sensors / Logs     Relay Control
       │                │
       ↓                ↓
   Dashboard          Bulb
