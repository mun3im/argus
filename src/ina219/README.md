# INA219 — Current & Power Monitor

Integration of the Adafruit INA219 high-side I²C current/power sensor for
real-time energy profiling of each inference pipeline stage.

## Files

| File | Description |
|------|-------------|
| `Guide_02_INA219_Current_Sensor.md` | Full component guide, wiring, energy accounting patterns |

## Role in System

The INA219 is placed in series with the Portenta H7 power rail and used to
measure mW-level power consumption during mel computation and TFLite inference.
Enables per-stage energy accounting: `Energy (µJ) = Power (mW) × Time (µs) / 1000`

## Full Documentation

→ [Guide 02 — INA219 Current Sensor](./Guide_02_INA219_Current_Sensor.md)
