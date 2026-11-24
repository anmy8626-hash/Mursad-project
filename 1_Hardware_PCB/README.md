# 1. Hardware & Embedded Layer (`1_Hardware_PCB`)

This directory contains the embedded firmware, register maps, and circuit schematics for **MURSAD** — an autonomous cyber-physical shield designed to protect critical OT/ICS infrastructure through real-time physical verification and sub-10 ms hardware isolation.

---

## 📁 Directory Structure

* **`firmware/mursad_esp32.ino`**: Core ESP32-S3 firmware handling sensor acquisition, local Digital Twin physics verification, serial JSON telemetry, and hardware relay tripping.
* **`registers.json`**: Standardized telemetry and control register mapping (Voltage, Current, Power, PF, Temperature, Pressure, Impedance, and Relay Trip states).
* **`schematics/`**: Circuit diagrams, pinout mappings, and opto-isolated relay protection layout.

---

## ⚙️ Core Hardware Architecture

1. **Microcontroller (Air-Gapped Edge):** `ESP32-S3` running locally without cloud or SCADA dependency.
2. **Physical Sensing:** Continuous acquisition of:
   * Electrical parameters: Voltage ($V$), Current ($I$), Active Power ($P$), and Power Factor ($PF$).
   * Environmental/Process parameters: Temperature ($^\circ\text{C}$) and Pressure.
3. **Hardware Isolation:** Opto-isolated relay driver capable of physically disconnecting the load in **< 10 ms** whenever physical telemetry deviates beyond **$\pm 5\%$** from the Digital Twin model.
4. **Backup Power:** Integrated Li-ion UPS circuit ensuring continuous monitoring and fail-safe isolation during main power blackouts.

---

## 🔌 Pinout & Wiring Summary

| Component / Signal | ESP32-S3 Pin | Notes |
| :--- | :--- | :--- |
| **Opto-Isolated Relay (Trip)** | `GPIO XX` | Active-HIGH/LOW hardware isolation (< 10 ms) |
| **Temperature Sensor** | `GPIO XX` | Analog / I2C / OneWire interface |
| **Voltage / Current Sensing** | `GPIO XX` | ADC / SPI / I2C telemetry input |
| **Status OLED Display (128×32)** | `SDA / SCL` | Local real-time status & trip alerts |
| **USB Serial Telemetry** | `UART0 (115200 baud)` | Air-gapped local dashboard communication |

*(Update `GPIO XX` to match your exact pin assignments in `mursad_esp32.ino`.)*

---

## 🚀 Quick Start (Flashing Firmware)

1. Open `firmware/mursad_esp32.ino` in **Arduino IDE** or **PlatformIO**.
2. Select board: **ESP32S3 Dev Module**.
3. Set Serial Monitor / Web Serial baud rate to **`115200`**.
4. Verify wiring against `schematics/` before powering the high-voltage/load side.

