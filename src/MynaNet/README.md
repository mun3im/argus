# MynaNet

Tier 2 species classifier — a MobileNet-style Depthwise-Separable CNN
(`MynaNet-1j`) that classifies a 64×300 mel spectrogram into 12 Malaysian
garden bird species. Trained on the MyGardenBird dataset. Runs on-demand,
only after DrongoNet (Tier 1) detects bird activity. See
[`Guide_04_MynaNet_Species_CNN.md`](Guide_04_MynaNet_Species_CNN.md) for
full architecture, DSP parameters, and integration details.

## Files

| File | Purpose |
|---|---|
| `MynaNet.ino` | Standalone smoke-test sketch — inference from a pre-computed mel header, no live microphone required |
| `Mynanet_1j.h` | INT8 TFLite Micro model (~267 KB Flash) |
| `mel_sample.h` | Pre-computed mel spectrogram sample for smoke testing |
| `Spotted_Dove_mel.h` | Pre-computed INT8 mel input — Spotted Dove |
| `Asian_Koel_mel.h` | Pre-computed INT8 mel input — Asian Koel |
| `MynaNet_1b_baseline.tflite` | Baseline `.tflite` model (pre-quantization reference) |

## Species (12 classes)

Coppersmith Barbet, Spotted Dove, Collared Kingfisher, Asian Koel,
White-breasted Waterhen, Common Iora, Large-tailed Nightjar, Yellow-vented
Bulbul, Pied Fantail, Olive-backed Sunbird, Common Tailorbird, and one
additional species — see the guide for the full labelled table.

## Performance

- 94.91% top-1 accuracy across 12 species
- <45 ms inference on Cortex-M7
- Unknown / low-confidence species → audio saved for review

## Requirements

- Arduino Portenta H7 (STM32H747XI)
- TensorFlow Lite Micro for Arduino
- CMSIS-DSP (bundled with Arduino Mbed OS Portenta Boards)
