# 🌸 Women Safety Band — Arduino Uno Project

## ⚙️ Overview

The Women Safety Band is a wearable smart security system built using Arduino Uno. It continuously monitors the environment around the wearer and provides real-time safety alerts through:

- Motion/Radar Detection (LD2410)
- Wire-cut/Tamper Detection
- GPS Location Tracking (NEO-6M)
- Emergency SMS Alerts (SIM900A GSM)
- Safe Mode Button (to deactivate alerts when removed intentionally)
- Live OLED Display & Serial Monitor feedback

This project aims to enhance personal safety by automatically sending distress alerts with GPS location whenever suspicious motion, tampering, or abnormal conditions are detected.

## 🧠 Features

| Feature | Description |
|---|---|
| 👥 Human Detection | Detects motion & counts number of people using LD2410 radar. |
| ⚠️ Wire Cut / Tamper Alert | Triggers buzzer & SMS alert if the safety band circuit is broken. |
| 📍 GPS Location | Sends live Google Maps link of user’s coordinates in SMS. |
| 📨 Automatic SMS Alerts | Sends alert messages to multiple pre-set emergency contacts. |
| 🔕 Safe Mode | Hold the button for 10 seconds to deactivate alerts (e.g., while bathing or removing the band). Hold again for 3 seconds to reactivate. |
| 🖥 OLED Display | Shows all important alerts, status updates, and mode changes. |
| 💡 Serial Monitor Logging | Every system action is also printed on Serial Monitor for debugging. |

## 🧩 Hardware Components

| Component | Quantity | Purpose |
|---|---:|---|
| Arduino Uno | 1 | Main microcontroller |
| LD2410 Radar Sensor | 1 | Human motion & presence detection |
| SIM900A GSM Module | 1 | Sends SMS alerts |
| NEO-6M GPS Module | 1 | Provides latitude & longitude |
| SSD1306 OLED Display (I²C) | 1 | Displays status & alerts |
| Buzzer | 1 | Audio alert |
| Push Button | 1 | Safe Mode toggle |
| Wire-cut Sensor (or circuit loop) | 1 | Detects tampering |
| LEDs | 1 | Visual status indicator |
| Power Supply | 5V DC | Power for all modules |

## 🔌 Wiring / Pin Connections

| Component | Arduino Uno Pin | Description |
|---|---|---|
| OLED SDA | A4 | I²C Data |
| OLED SCL | A5 | I²C Clock |
| LD2410 TX | D2 | Radar data (SoftwareSerial RX) |
| LD2410 OUT | D4 | Presence detection (digital) |
| Wire Sensor | D5 | Detects wire break (Active LOW) |
| Buzzer | D6 | Alarm output |
| SIM900A TX | D7 | GSM → Arduino |
| SIM900A RX | D8 | Arduino → GSM |
| GPS TX | D9 | GPS → Arduino |
| GPS RX | D10 | Arduino → GPS |
| Safe Mode Button | A0 | Press & hold (10s deactivate / 3s reactivate) |
| LED | D13 | Status LED |
| Power | 5V / GND | Common Power & Ground |

## 📦 Libraries Required

Install these from Arduino IDE → Tools → Manage Libraries:

- U8g2 by olikraus
- TinyGPS++ by Mikal Hart
- (Optional) SoftwareSerial (included by default)

## ⚙️ Setup Instructions

1. Connect all components as per the wiring table.
2. Open `Women_Safety_Band_Final.ino` in Arduino IDE.
3. Select:
	- Board: Arduino Uno
	- Port: COMx (your Arduino port)
4. Upload the sketch.
5. Open Serial Monitor at 9600 baud.

You should see initialization logs for radar, GSM, and GPS.

## 🧭 How It Works

**Startup Phase**

- Calibrates radar baseline energy.
- Connects GSM network and initializes OLED.

**Monitoring Mode**

- Continuously reads radar energy and presence pin.
- If multiple people or high energy is detected → sends alert SMS.
- If the wire loop is cut → buzzer + SMS alert + GPS location.

**Safe Mode**

- Long-press button (10s) → disables alerts.
- Short-press (3s) → reactivates system.

## SMS Message Example

```
🚨 ALERT: Wire Cut!
📍 https://maps.google.com/?q=23.456789,87.123456
```

## 🖥 OLED Display Messages

| Display Message | Meaning |
|---|---|
| “Smart Band Initializing…” | Booting up |
| “Calibration Done ✅” | Radar calibrated |
| “System Ready ✅” | Device active |
| “⚠ ALERT: Wire Cut!” | Tampering detected |
| “People Nearby: Multiple” | Motion detected |
| “SAFE MODE: Alerts Off” | Band temporarily deactivated |

## 🚨 Safety & Power Notes

- SIM900A GSM needs a stable 5V 2A power supply — don’t power it directly from the Arduino’s USB 5V pin.
- Use a separate regulated 5V adapter or a Li-ion battery (7.4V) + buck converter.
- Connect all GNDs together.
- Make sure GPS has a clear view of the sky for location fix.

## 🧰 Future Improvements

- Add voice assistant / mic trigger for SOS activation.
- Integrate vibration motor feedback.
- Add LoRa / Bluetooth for short-range communication.
- Create a mobile app to receive alerts and live tracking.

## 🏁 Summary

The Women Safety Band is a compact, low-cost, and efficient wearable safety device combining radar, GPS, and GSM for proactive security monitoring. It’s ideal for students, hackathons, and community safety projects focusing on hardware-based innovation for personal security.

**Created by:**

👩‍💻 Shirshak Mondal
🛠️ Powered by Arduino Uno & U8g2 OLED
💡 “Technology for Safety & Empowerment.”