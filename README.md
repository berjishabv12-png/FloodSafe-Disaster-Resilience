# FloodSafe — Flood-Aware Road Monitoring System

FloodSafe is an educational IoT prototype. An HC-SR04 ultrasonic sensor measures distance to a water surface. An ESP32 sends readings over Wi-Fi to a local Flask server, and a web dashboard displays the latest reading and status.

> **Safety:** This is not a certified flood-warning system. One sensor only measures its own location. Treat missing/stale data as UNKNOWN, not safe. The thresholds below are demonstration values only.

## Problem
During heavy rainfall, roads may become waterlogged or submerged. Road users may lack timely, location-specific information about water levels and possible hazards.

## Objectives
- Measure distance to the water surface.
- Send readings from ESP32 to a server over Wi-Fi.
- Display readings, status and update time on a dashboard.
- Mark readings UNKNOWN if updates stop.

## Workflow
1. HC-SR04 measures distance to the water surface.
2. ESP32 calculates distance and sends it to the Flask server.
3. The server estimates depth using a known sensor-to-reference height.
4. The dashboard refreshes and shows the latest reading and status.
5. If no recent update arrives, the status becomes UNKNOWN.

## Components
- ESP32 DevKit V1
- HC-SR04 ultrasonic sensor
- Three 1 kΩ resistors, breadboard and jumper wires
- Computer with Python and Flask
- Wi-Fi network shared by ESP32 and computer
- Optional ESP32-CAM for separate image capture

## Wiring
| HC-SR04 | ESP32 | Notes |
|---|---|---|
| VCC | VIN/5V, if supported by board | Power off before wiring |
| GND | GND | Common ground required |
| TRIG | GPIO 5 | Trigger |
| ECHO | GPIO 18 through divider | Never connect ECHO directly to GPIO |

### Three-resistor ECHO voltage divider
- HC-SR04 ECHO → R1 (1 kΩ) → junction.
- Junction → ESP32 GPIO 18.
- Junction → R2 (1 kΩ) in series with R3 (1 kΩ) → GND.
- This gives 1 kΩ on the upper leg and 2 kΩ to GND, reducing an approximately 5 V signal to about 3.3 V.
- Disconnect USB/power while wiring.

## Software setup
Install Python 3 and Arduino IDE with ESP32 board support. In a terminal inside this folder, install Flask:

```bash
python -m pip install -r requirements.txt
python app.py
```

Open `http://127.0.0.1:5000` on the computer. To view it from another device on the same Wi-Fi, use `http://YOUR_COMPUTER_IP:5000`.

On Windows, run `ipconfig` to find the computer's Wi-Fi IPv4 address. Replace `192.168.1.10` in `esp32_floodsafe.ino` with that address. Do not use `127.0.0.1` in the ESP32 sketch.

## Configure ESP32
In `esp32_floodsafe.ino`, set:
- `WIFI_SSID` and `WIFI_PASSWORD`
- `SERVER_URL` to `http://YOUR_COMPUTER_IP:5000/api/update`
- `SENSOR_HEIGHT_CM` to the measured distance from sensor face to the container bottom/reference when empty.

Upload the sketch and open Serial Monitor at 115200 baud. The ESP32 and computer must be on the same Wi-Fi network. Allow Python through the private-network firewall if prompted.

## Depth calculation and calibration
Estimated depth = fixed sensor-to-reference height − measured distance to water. Measure the reference height with a ruler. Test at known water depths and compare results. If distance exceeds the reference height, the demo reports zero estimated depth; this is not proof of safety. No-echo readings are not submitted.

## Demo thresholds
- Below 5 cm: LOW
- 5–15 cm: CAUTION
- Above 15 cm: HIGH WATER
- No recent data: UNKNOWN

These are examples, not official road-safety standards. Calibrate for the actual site.

## API
| Method | Endpoint | Purpose |
|---|---|---|
| GET | `/` | Dashboard |
| GET | `/api/roads` | Latest readings |
| POST | `/api/update` | Receive ESP32 reading |

Example payload: `{"road_id":"ROAD_1","distance_cm":18.4}`

## Test checklist
1. Check wiring and common ground.
2. Compare sensor distance with a ruler at known distances.
3. Test depth calculation with a container and known water levels.
4. Confirm Wi-Fi and HTTP response in Serial Monitor.
5. Confirm dashboard readings update.
6. Stop ESP32 updates and verify the status becomes UNKNOWN.
7. Repeat readings and record variations.

## Limitations
- HC-SR04 is not waterproof; protect it from rain and splashes.
- Ripples, foam, angled surfaces, obstructions and vibration can affect readings.
- Wi-Fi, power or server failure can interrupt updates.
- A single sensor cannot verify an entire road.
- The starter does not calculate real alternative routes or verify map routes.
- The local HTTP server is for controlled demonstrations, not public deployment.

## Future improvements
Use an outdoor-rated waterproof sensor and enclosure; add battery/solar power, persistent storage, multiple sensor nodes, sensor-health alerts, and notifications. Only add route guidance using verified road maps and authoritative closure information. Obtain permissions and validate the system before field deployment.

## UN Sustainable Development Goals
- **SDG 11 — Sustainable Cities and Communities:** supports exploration of safer, more resilient communities.
- **SDG 13 — Climate Action:** supports preparedness for flood-related risks.

## Repository contents
- `README.md` — project overview and instructions
- `app.py` — Flask server and API
- `esp32_floodsafe.ino` — ESP32 firmware
- `templates/index.html` — live dashboard
- `requirements.txt` — Python dependency

Sample thresholds and screenshots are illustrative unless confirmed by live sensor readings.
