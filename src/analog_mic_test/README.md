# analog_mic_test

Standalone smoke test for the MAX4466 electret microphone amplifier
(analog input) on the Arduino Portenta H7. Verifies wiring and ADC
response before the microphone is wired into the DSP mel-spectrogram
pipeline used by the other sketches. See
[`Guide_01_Microphone.md`](Guide_01_Microphone.md) for wiring, library
setup, and tuning/calibration details.

## Files

| File | Purpose |
|---|---|
| `analog_mic_test.ino` | Reads the MAX4466 analog output and flashes the built-in RGB LEDs (green on sound detected, blue when idle) |

## Key specs

| Parameter | Value |
|---|---|
| Supply voltage | 3.3 V (never use 5 V — Portenta ADC is 3.3 V tolerant) |
| ADC resolution | 12-bit (0–4095), midpoint ≈ 2048 |
| Output pin | Any of A0–A7 (must match the sketch's `microphonePin`) |

## Requirements

- Arduino Portenta H7 (STM32H747XI)
- Arduino Mbed OS Portenta Boards package (no external library needed for `analogRead()`)
