# ARGUS

Full two-tier cascade firmware for the Arduino Portenta H7.

`ARGUS.ino` combines both models into a single sketch: **DrongoNet-micro**
(Tier 1, Cortex-M4-style always-on bird activity detector) gates a wake-up
into **MynaNet** (Tier 2, species classifier). This is the production
integration sketch — see [`../DrongoNet`](../DrongoNet) and
[`../MynaNet`](../MynaNet) for the individual models and their standalone
benchmark sketches.

## Files

| File | Purpose |
|---|---|
| `ARGUS.ino` | Main sketch — audio capture, dual mel-spectrogram pipelines, cascaded inference, INA219 power logging |
| `drongonet-micro.h` | Tier 1 model (INT8 TFLite Micro, 919 params) |
| `drongonet_mel_tables.h` | Mel filterbank tables for DrongoNet (16 bins, 1024-pt FFT) |
| `Mynanet_1j.h` | Tier 2 model (INT8 TFLite Micro DS-CNN) |
| `mynanet_mel_tables_64.h` | Mel filterbank tables for MynaNet (64 bins, 512-pt FFT) |

## Pipeline

1. Capture 3 s / 16 kHz audio window (`audioBuffer`)
2. Compute 16×184 mel spectrogram → DrongoNet-micro → `P(bird)`
3. If bird activity detected, compute 64×300 mel spectrogram → MynaNet → species probabilities
4. Log per-stage timing (DWT cycle counter) and power draw (INA219, if present)

## Requirements

- Arduino Portenta H7 (STM32H747XI)
- Arduino Mbed OS Portenta Boards package
- TensorFlow Lite Micro for Arduino
- Adafruit INA219 library (optional, for power logging)

## See also

- [`../DrongoNet/Guide_03_DrongoNet_BAD_CNN.md`](../DrongoNet/Guide_03_DrongoNet_BAD_CNN.md)
- [`../MynaNet/Guide_04_MynaNet_Species_CNN.md`](../MynaNet/Guide_04_MynaNet_Species_CNN.md)
- [`../ina219/Guide_02_INA219_Current_Sensor.md`](../ina219/Guide_02_INA219_Current_Sensor.md)
