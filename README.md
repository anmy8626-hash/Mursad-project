# MURSAD — Cyber-Physical Digital Twin Shield for Critical Infrastructure

**MURSAD (مرصاد)** is an edge-based cyber-physical protection and anomaly detection system designed for Operational Technology (OT) and electrical substations. Instead of relying solely on software-level network commands, MURSAD cross-verifies live electrical and thermal measurements against a local **Digital Twin** and **Gaussian Mixture Model (GMM)**—triggering deterministic **hardware-level isolation in < 10 ms** whenever physical deviation exceeds **±5%**.

---

## System Architecture

MURSAD is structured into three integrated defense layers:

| Layer | Directory | Core Stack | Primary Role |

| **1. Hardware & Embedded Control** | [`1_Hardware_PCB/`](./1_Hardware_PCB) | ESP32-S3, Opto-Isolated Relays, Sensors | Real-time telemetry acquisition and deterministic hardware load isolation (< 10 ms) independent of compromised IT/OT commands. |
| **2. Digital Twin & Edge AI** | [`2_AI_DigitalTwin/`](./2_AI_DigitalTwin) | Python, C++ (`MursadCorrectedGMM.h`), 10-Feature GMM | Physics-based impedance verification (**±5%** threshold) and statistical anomaly scoring (NLL threshold `33.94`). |
| **3. OT Network & Monitoring** | [`3_OT_Network_IT/`](./3_OT_Network_IT) | Modbus TCP (`pymodbus`), Node-RED HMI | Industrial communication simulation, fault/spoofing test cases, and real-time operator dashboard. |



## Key Engineering Highlights

* **Sub-10 ms Hardware Isolation:** Opto-isolated relay tripping controlled directly by embedded logic on the `ESP32-S3` ensures physical protection even if the network layer is compromised.
* **Physics-Informed Digital Twin (±5% Guardrail):** Continuously compares expected vs. measured electrical impedance ($Z = V / I$) and thermal behavior to detect command-to-physics mismatches.
* **10-Feature GMM Anomaly Detection:** Evaluates *Voltage, Current, Power Factor, Temperature, Pressure, Active Power, Apparent Power, Impedance, Thermal Index,* and *Electro-Mechanical Ratio* against a calibrated threshold (`33.9377`).
* **Safety Validation Interlock:** Automated AI-driven isolation remains locked by default (`false`) until physical sensor baselines and relay polarities are verified on the hardware testbed.



## Quick Start
1. **Run the Digital Twin & GMM Anomaly Evaluator:**
   python 2_AI_DigitalTwin/models/anomaly_detector.py
1. **Run the Digital Twin & GMM Anomaly Evaluator:**
   ```bash
   python 2_AI_DigitalTwin/models/anomaly_detector.py
