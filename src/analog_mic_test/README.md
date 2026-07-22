# Analog Mic Test — MAX4466 Smoke Test

Standalone smoke test sketch to verify MAX4466 microphone wiring and
ADC response before running the full inference pipeline.

## Files

| File | Description |
|------|-------------|
| `analog_mic_test.ino` | Smoke test sketch — blinks LED on sound detection |
| `Guide_01_Microphone.md` | Full component guide, wiring, calibration, troubleshooting |

## Quick Check

Upload `analog_mic_test.ino` to the Portenta H7 (M7 core):
- 🔵 **Blue LED** → quiet / idle
- 🟢 **Green LED** → sound detected (delta > 1000 ADC counts)

## Full Documentation

→ [Guide 01 — Microphone](./Guide_01_Microphone.md)
