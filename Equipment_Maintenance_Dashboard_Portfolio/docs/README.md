# Equipment Maintenance Dashboard

A real-time equipment monitoring system built with **Arduino**, **Python**, and **Excel VBA**.  
The project simulates an industrial preventive maintenance solution by tracking equipment temperature, vibration events, and operating hours — then visualizing and logging the data for scheduling service and minimizing downtime.

---

## Features

- **Arduino-based sensor hub** with:
  - Temperature sensing (DHT11 / thermistor)
  - Vibration detection (tilt sensor)
  - Equipment runtime tracking (via toggle switch)
  - On-device LCD status display
- **Python serial interface** to:
  - Read live sensor data from Arduino
  - Append readings to an Excel file
- **Excel VBA dashboard** to:
  - Auto-refresh and visualize sensor data
  - Highlight equipment needing urgent maintenance
  - Track hours since last service

---

## Hardware Setup

- **Arduino Uno**
- **DHT11** temperature & humidity sensor (temperature reading used)
- **Tilt switch** for vibration event detection
- **Potentiometer** for LCD contrast adjustment
- **1602 LCD** (parallel mode)
- **Push button** to toggle equipment state
- Breadboard & jumper wires

---


## Photos

### Hardware Build
![Hardware Setup](images/Hardware_Setup.jpg)

### Excel Dashboard
![Excel Dashboard](images/excel_dashboard.png)

### Live Command Line Data
![Command Line Output](images/cmd_dashboard.png)

---

## How It Works

1. **Arduino** reads temperature, vibration, and equipment status.
2. Data is sent via USB serial to **Python**.
3. Python script logs readings to an **Excel workbook**.
4. **Excel VBA** highlights maintenance priorities using conditional formatting.

---

## Sample Data

| AssetID | Timestamp           | Temp_C | Vibration_Events | Equipment_On | Runtime_Minutes | Hours_Since_LastService |
|---------|---------------------|--------|------------------|--------------|-----------------|-------------------------|
| A1000   | 2025-08-12 04:19    | 24.6   | 0                | FALSE        | 0.00            | 0.00                    |
| A1000   | 2025-08-12 04:20    | 24.5   | 2                | TRUE         | 0.29            | 0.01                    |
| A1000   | 2025-08-12 04:21    | 24.4   | 0                | TRUE         | 1.95            | 0.03                    |

---

## Skills Demonstrated

- Arduino programming (C/C++)
- Sensor integration
- LCD interfacing
- Serial communication (Arduino ↔ Python)
- Data logging & Excel automation
- Preventive maintenance simulation

---

## Future Improvements

- Replace tilt sensor with accelerometer for better vibration detection
- Integrate cloud dashboard (e.g., ThingSpeak or Grafana)
- Add wireless communication (ESP8266 / ESP32)
- Expand to multiple assets with ID tracking

---