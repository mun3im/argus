# INA219

Documentation for the Adafruit INA219 high-side I²C current/power monitor,
used for real-time energy profiling of each inference pipeline stage (mel
computation, DrongoNet, MynaNet) across the ARGUS sketches. See
[`Guide_02_INA219_Current_Sensor.md`](Guide_02_INA219_Current_Sensor.md)
for wiring, library setup, a smoke test, and integration examples.

This directory holds documentation only — the INA219 driver code
(`Adafruit_INA219` + `Wire`) is integrated directly into the sketches under
[`../ARGUS`](../ARGUS), [`../DrongoNet`](../DrongoNet), and
[`../MynaNet`](../MynaNet).

## Key specs

| Parameter | Value |
|---|---|
| Interface | I²C (address `0x40` default) |
| Voltage measurement | Up to 26 V bus voltage |
| Current measurement | ±3.2 A (with 0.1 Ω shunt) |
| Supply voltage | 3.0–5.5 V (use 3.3 V with Portenta H7) |

## Requirements

- Adafruit INA219 library
- Arduino Portenta H7 (STM32H747XI)
