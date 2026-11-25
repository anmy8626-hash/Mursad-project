# 3. OT Network, Modbus Simulation & Monitoring (3_OT_Network_IT)

This layer covers industrial OT/IT communication simulation, Modbus TCP monitoring, fault/attack testing scenarios, and operator dashboard flows for **MURSAD**

## Key Components
plc_server.py`: Simulated Modbus TCP device/server exposing holding registers (e.g., voltage, current, temperature) for telemetry and control testing.
attacker_client.py: Client/test script for querying registers and testing anomalous or unauthorized command/telemetry scenarios.
dashboard_config.json: Node-RED flow (`flows.json`) / dashboard configuration for visualizing electrical and thermal telemetry.
https://mursad-9u4oqhrp.manus.space/
