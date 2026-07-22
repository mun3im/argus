# DrongoNet

Tier 1 Bird Activity Detector (BAD) — an ultra-compact CNN that answers a
binary question, `P(bird)` vs `P(no_bird)`, from a 16×184 mel spectrogram.
Trained on the SEABAD dataset (Southeast Asia Bird Activity Detection).
See [`Guide_03_DrongoNet_BAD_CNN.md`](Guide_03_DrongoNet_BAD_CNN.md) for
full architecture, DSP parameters, and integration details.

## Variants

| Variant | Model | Mel bins | Flash | Notes |
|---|---|---|---|---|
| [`DrongoNet_Micro`](DrongoNet_Micro) | drongonet-micro | 16 | 5.2 KB | Folded INT8 graph (no explicit QUANTIZE/DEQUANTIZE nodes) |
| [`DrongoNet_Nano`](DrongoNet_Nano) | drongonet-nano | 16 | 5.4 KB | Non-folded INT8 graph (explicit boundary nodes) |
| [`DrongoNet_Edge`](DrongoNet_Edge) | drongonet-edge | 80 | larger | Wider mel filterbank (`fmin=100`, `fmax=8000`) for higher-resolution benchmarking |

`DrongoNet_Micro` (919 parameters, AUC 0.9741 on the SEABAD benchmark) is
the variant used in the production [`../ARGUS`](../ARGUS) cascade sketch.
`DrongoNet_Nano` and `DrongoNet_Edge` are standalone benchmark sketches for
comparing op-resolver and mel-resolution trade-offs.

## Requirements

- Arduino Portenta H7 (STM32H747XI)
- TensorFlow Lite Micro for Arduino
- CMSIS-DSP (bundled with Arduino Mbed OS Portenta Boards)
