# EnviroSense

**Project Overview**

EnviroSense is an open hardware and software project that measures local environmental conditions (temperature, humidity, pressure, and gas/air-quality) using a BME680 sensor and an ESP32-based microcontroller. It can display readings on an OLED, log data to local storage for low-power scenarios, and send telemetry to a cloud endpoint for visualization and analysis.

**Key Features**
- **Sensors**: Temperature, Humidity, Pressure, Gas Resistance (BME680).
- **Display**: 128x64 OLED for real-time readout and status.
- **Connectivity**: Wi‑Fi and optional MQTT/HTTP telemetry to send data to a backend.
- **Power Options**: Low-power deep sleep logging with LittleFS support for intermittent uploads.
- **PCB & Schematics**: KiCad project files and PCB artwork included.

**Why this project**
EnviroSense is suitable for hobbyists, students, and developers who want a compact weather + air-quality monitor that can run standalone, display data locally, and integrate with cloud dashboards (Node‑RED, Grafana, custom backends).

**Repository Layout**
- **Code**: [code/](code/) — Arduino sketches and modules. Notable files:
	- [code/FINAL_CODE.ino](code/FINAL_CODE.ino) — Full BME680 + OLED + HTTP uploader sketch.
	- [code/Main.ino](code/Main.ino) — Alternative sketch using MQTT, LCD, and ACS712 current sensing.
	- [code/interface/interface.ino](code/interface/interface.ino) — OLED interface example.
	- [code/OLED/OLED.ino](code/OLED/OLED.ino) — OLED display example.
	- [code/Power Optimization and storage/powerAndStorage.ino](code/Power%20Optimization%20and%20storage/powerAndStorage.ino) — Low-power logging + LittleFS.
- **PCB & Schematics**: [Pcb/](Pcb/) and [circuit diagram/](circuit%20diagram/) contain KiCad schematics and PCB files.
- **Images**: [images/](images/) — photos, screenshots, and wiring diagrams.
- **Task.md**: [Task.md](Task.md) — project notes (currently empty).

**Hardware Required (example)**
- ESP32 development board (any flavor with required I/O).
- BME680 sensor module (I2C).
- 128x64 I2C OLED display (SSD1306 compatible).
- Optional: ACS712 current sensor, power management components, battery for low-power tests.

**Software & Libraries**
- Arduino IDE or PlatformIO.
- Libraries commonly used in the sketches:
	- `Adafruit_BME680`, `Adafruit_Sensor`
	- `Adafruit_SSD1306`, `Adafruit_GFX`
	- `ArduinoJson` (for HTTP payloads)
	- `PubSubClient` (for MQTT in `Main.ino`)
	- `LittleFS` (for local logging)

**Quick Start (Arduino IDE)**
1. Install the required libraries listed above via the Library Manager.
2. Open one of the sketches in `code/` (for example, [code/FINAL_CODE.ino](code/FINAL_CODE.ino)).
3. Update Wi‑Fi credentials and server URL (if using HTTP upload).
4. Select the correct ESP32 board and COM port, then upload.

**Usage Tips**
- For continuous cloud upload use `FINAL_CODE.ino` and configure `serverURL`.
- For low-power battery operation use the deep-sleep logging sketch in `Power Optimization and storage`.
- For MQTT streaming and additional sensors (current sensing), see `Main.ino`.

**Where to look next**
- Schematics and board files: [circuit diagram/](circuit%20diagram/) and [Pcb/Envirosense/](Pcb/Envirosense/).
- Images and screenshots: [images/](images/).

**Contributing**
- Fork the repo, add features or fixes in `code/`, and file a pull request with a short description of changes.

**License**
- This repository does not currently include a license file. If you want it to be open-source, add a `LICENSE` (MIT, Apache-2.0, etc.) and update this README.


