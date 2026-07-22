# ARGUS — Autonomous Real-time Guardian for Ultra-low-power Species Monitoring

The production sketch for the two-tier bird detection and classification system
running on the Arduino Portenta H7 (STM32H747XI dual-core).

## Files

| File | Description |
|------|-------------|
| `ARGUS.ino` | Main sketch — FreeRTOS tasks, RPC inter-core, full cascade pipeline |
| `drongonet-micro.h` | DrongoNet Micro INT8 model weights (Tier 1 BAD detector) |
| `drongonet_mel_tables.h` | Pre-computed mel filterbank for DrongoNet (16-bin, 16 kHz) |
| `Mynanet_1j.h` | MynaNet 1j INT8 model weights (Tier 2 species classifier) |
| `mynanet_mel_tables_64.h` | Pre-computed mel filterbank for MynaNet (64-bin, 16 kHz) |

## How It Works

- **Cortex-M4 (Tier 1):** Runs DrongoNet every 3 seconds to detect bird activity
- **Cortex-M7 (Tier 2):** Wakes on positive detection, runs MynaNet for species ID
- Inter-core communication via OpenAMP RPC

## Full Documentation

→ [ARGUS System Architecture (root README)](../../README.md)
