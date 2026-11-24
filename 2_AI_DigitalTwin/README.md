# 2. Digital Twin & Physical Verification Layer (`2_AI_DigitalTwin`)

This layer provides the physics-based **Digital Twin** verification and anomaly detection logic for **MURSAD**. Instead of trusting control commands or digital telemetry blindly, MURSAD cross-checks live physical telemetry against expected physical behavior and supports sub-10 ms hardware isolation when deviation exceeds **±5%**.

## Directory Structure
* `models/`: Anomaly detection and Digital Twin reference scripts (e.g., `anomaly_detector.py`).
* `datasets/`: Telemetry samples, normal operating baselines, and fault/tamper test datasets.
* `README.md`: Documentation of verification logic, deviation thresholds, and model structure.

## How Verification Works
1. **Expected Physical Behavior:** Computes expected electrical/thermal parameters (such as impedance $Z = V / I$ and load behavior) from active system state.
2. **Deviation Monitoring:** Measures percentage deviation between actual readings and expected Digital Twin values against the **±5%** safety limit.
3. **Isolation & Forensics:** Flags anomalies (e.g., physical deviation > ±5%, over-temperature, or command-to-physics mismatch) to trigger hardware isolation (< 10 ms) and log events.

