# MynaNet — Species Classifier (Tier 2 Classifier)

MynaNet is a Depthwise Separable CNN with Squeeze-and-Excitation and attention
blocks, trained to classify 12 Southeast Asian bird species from 64-mel spectrograms.

## Files

| File | Description |
|------|-------------|
| `MynaNet.ino` | Standalone inference sketch (Cortex-M7) |
| `Mynanet_1j.h` | INT8 model weights (~267 KB) |
| `MynaNet_1b_baseline.tflite` | Floating-point baseline model |
| `mel_sample.h` | Sample mel input for offline unit testing |
| `Asian_Koel_mel.h` | Asian Koel test mel spectrogram |
| `Spotted_Dove_mel.h` | Spotted Dove test mel spectrogram |

## Performance

- 92% top-1 accuracy (12-class)
- <45 ms inference on Cortex-M7 @ 480 MHz
- Input: 64×300 mel spectrogram (n_fft=1024, 16 kHz)

## Full Documentation

→ [Guide 04 — MynaNet Species CNN](./Guide_04_MynaNet_Species_CNN.md)
