# Guide 03 — DrongoNet Bird Activity Detector (BAD) CNN
## Arduino Portenta H7 Bird Recognition System

> **Model**: drongonet-micro / drongonet-nano (INT8 Quantised)  
> **Task**: Binary Bird Activity Detection — outputs `P(bird)` and `P(no_bird)`  
> **Input**: 16-bin × 184-frame Mel Spectrogram, sampled at 16 kHz  
> **Hardware**: Arduino Portenta H7 (STM32H747XI, Cortex-M7 @ 480 MHz)  
> **AUC**: 0.9741 (DRONGO dataset, 1677 species, 50k clips)  

---

## Table of Contents

1. [Model Overview](#1-model-overview)
2. [Library Installation](#2-library-installation)
3. [Smoke Test — Minimal CNN Inference Validation](#3-smoke-test--minimal-cnn-inference-validation)
4. [Integration — Full CMSIS-DSP + DrongoNet Pipeline](#4-integration--full-cmsis-dsp--DrongoNet-pipeline)
5. [Memory Architecture](#5-memory-architecture)
6. [Output Decoding and Temporal Smoothing](#6-output-decoding-and-temporal-smoothing)
7. [Troubleshooting](#7-troubleshooting)

---

## 1. Model Overview

### 1.1 Architecture

drongonet-micro is an ultra-compact CNN with only **919 parameters** (5.2 KB in INT8 flash). Despite its tiny size it achieves AUC 0.9741 on the South-East Asia Bird Activity Detection (DRONGO) benchmark.

| Layer | Output Shape | Parameters |
|---|---|---|
| Input | [1, 184, 16, 1] | — |
| FrequencyEmphasis | [1, 184, 16, 1] | 17 |
| Conv2D (6 filters, 3×3, ReLU) | [1, 184, 16, 6] | 336 |
| MaxPooling2D (2×2) | [1, 92, 8, 6] | — |
| Conv2D (12 filters, 3×3, ReLU) | [1, 92, 8, 12] | 660 |
| Conv2D PW (12 filters, 1×1) | [1, 92, 8, 12] | — |
| GlobalAveragePooling2D | [1, 12] | — |
| Dropout (p=0.1) | [1, 12] | — |
| Dense (2, Softmax) | [1, 2] | 26 |

**Total: 919 parameters | Flash: 5.2 KB (micro) / 5.4 KB (nano)**

### 1.2 Mel Spectrogram Parameters

The model was trained with these exact DSP parameters — the on-device `computeMelSpectrogram()` must reproduce them identically:

| Parameter | Value | Notes |
|---|---|---|
| Sample rate | 16,000 Hz | |
| Audio duration | 3 seconds | 48,000 samples |
| n_fft | 1024 | 64 ms window |
| hop_length | 256 | 16 ms hop |
| n_mels | 16 | Ultra-low resolution |
| fmin | 100 Hz | Low-pass start |
| fmax | 8,000 Hz | Nyquist limit |
| Time frames | 184 | ⌊(48000 − 1024) / 256⌋ + 1 |
| center | False | Deterministic boundaries |

### 1.3 Model Variants

| Variant | File | Size | Op Resolver | Notes |
|---|---|---|---|---|
| drongonet-micro | `drongonet-micro.h` | 5.2 KB | `MicroMutableOpResolver<6>` | Folded (no QUANTIZE/DEQUANTIZE nodes) |
| drongonet-nano | `drongonet-nano.h` | 5.4 KB | `MicroMutableOpResolver<8>` | Non-folded (explicit boundary nodes) |
| BAD_tiny_model | `BAD_tiny_model.h` | 75 KB | `AllOpsResolver` | Minimal smoke-test model |

---

## 2. Library Installation

### 2.1 TensorFlow Lite Micro

**Install via Arduino Library Manager:**

1. **Tools → Manage Libraries…**
2. Search: `TensorFlowLite`
3. Install: **Arduino_TensorFlowLite** by TensorFlow Authors (≥ 2.4.0)

This provides all required TFLM headers:

```cpp
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
```

### 2.2 CMSIS-DSP (Hardware FFT — Bundled)

The mel spectrogram computation uses `arm_rfft_fast_f32` from CMSIS-DSP. **No installation needed** — it is bundled with the Mbed OS board package:

```cpp
#include <arm_math.h>   // Automatically available after board package install
```

### 2.3 Mbed Runtime Stats (Bundled)

Memory profiling uses Mbed OS statistics. Also bundled — no install:

```cpp
#include <mbed.h>
#include <mbed_stats.h>
```

### 2.4 `abs` Macro Fix

Arduino.h defines `abs()` as a C macro that conflicts with `std::abs` overloads in `<cmath>`. **Always add this before including `<cmath>`**:

```cpp
#ifdef abs
#undef abs
#endif
#include <cmath>
```

This is already present at the top of every sketch in this project.

---

## 3. Smoke Test — Minimal CNN Inference Validation

### 3.1 Purpose

Confirm that the DrongoNet model loads from Flash, the tensor arena allocates without error, and the inference loop produces valid changing outputs from the microphone in real time.

### 3.2 Smoke Test Sketch

> **Source file:** [`src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino`](../src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino)

This sketch uses a simplified RMS-based mel approximation (not the full CMSIS-DSP pipeline) to validate the inference stack in isolation from the DSP complexity.

```cpp
// ================================================================
// Smoke Test: DrongoNet CNN Inference
// Uses BAD_tiny_model (75 KB) with a simplified mel approximation.
// Purpose: Validate TFLite Micro loads, AllocateTensors succeeds,
//          and Invoke() produces changing output from the mic.
//
// Expected setup output:
//   [OK] Model schema version matches.
//   [OK] Input tensor: 1536 B, Output tensor: 2 B
//   [OK] Arena used: ~30000 B
//   [READY] Inference engine ready.
// ================================================================

#include "Arduino.h"
#ifdef abs
#undef abs
#endif

#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

#include "BAD_tiny_model.h"    // Model must be in same folder as this .ino

// ── Mic and audio settings ───────────────────────────────────────
const int micPin      = A0;
const int SAMPLE_RATE = 16000;
const int BUFFER_SIZE = SAMPLE_RATE * 1;   // 1-second window
int16_t   audioBuffer[BUFFER_SIZE];
int       bufferIndex = 0;

// ── TFLite Micro runtime objects ─────────────────────────────────
namespace {
  tflite::MicroErrorReporter  micro_error_reporter;
  tflite::ErrorReporter       *error_reporter = &micro_error_reporter;
  const tflite::Model         *model     = tflite::GetModel(bad_tiny_model);
  tflite::AllOpsResolver       resolver;
  tflite::MicroInterpreter    *interpreter = nullptr;
  TfLiteTensor                *input  = nullptr;
  TfLiteTensor                *output = nullptr;

  constexpr int kTensorArenaSize = 150 * 1024;
  alignas(16) uint8_t tensor_arena[kTensorArenaSize];
}

// ── Simplified mel (RMS-based approximation — smoke test only) ───
void computeMel(int16_t *audio, int length,
                float *melOut, int melBins = 16, int frames = 96) {
  int hop = length / frames;
  for (int f = 0; f < frames; f++) {
    for (int m = 0; m < melBins; m++) {
      float sum = 0.0f;
      int startIdx = f * hop + m * (hop / melBins);
      for (int j = startIdx; j < startIdx + hop / melBins && j < length; j++) {
        float s = audio[j] / 32768.0f;
        sum += s * s;
      }
      melOut[f * melBins + m] = sqrtf(sum / (hop / melBins + 1));
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial) { ; }

  Serial.println("=== DrongoNet BAD CNN Smoke Test ===");

  // 1. Schema version check
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.print("[FAIL] Schema mismatch: model="); Serial.print(model->version());
    Serial.print(" expected="); Serial.println(TFLITE_SCHEMA_VERSION);
    while (1) { }
  }
  Serial.println("[OK]  Model schema version matches.");

  // 2. Build interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // 3. Allocate tensors
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("[FAIL] AllocateTensors() failed — arena too small.");
    while (1) { }
  }

  input  = interpreter->input(0);
  output = interpreter->output(0);

  Serial.print("[OK]  Input tensor : "); Serial.print(input->bytes);  Serial.println(" B");
  Serial.print("[OK]  Output tensor: "); Serial.print(output->bytes); Serial.println(" B");
  Serial.print("[OK]  Arena used   : "); Serial.print(interpreter->arena_used_bytes());
  Serial.println(" B");
  Serial.println("[READY] Inference engine ready. Listening...\n");
}

void loop() {
  // 1. Sample 1 second of audio
  for (int i = 0; i < BUFFER_SIZE; i++) {
    int val       = analogRead(micPin);
    float sample  = ((float)val - 512.0f) / 512.0f;
    audioBuffer[bufferIndex++] = (int16_t)(sample * 32767);
    if (bufferIndex >= BUFFER_SIZE) bufferIndex = 0;
  }

  // 2. Compute simplified mel (smoke test — not production FFT)
  float mel[96 * 16];
  computeMel(audioBuffer, BUFFER_SIZE, mel, 16, 96);

  // 3. Quantise and fill input tensor
  for (int i = 0; i < 96 * 16; i++) {
    float scaled = mel[i] / input->params.scale + input->params.zero_point;
    if (scaled >  127.0f) scaled =  127.0f;
    if (scaled < -128.0f) scaled = -128.0f;
    input->data.int8[i] = (int8_t)scaled;
  }

  // 4. Run inference
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("[FAIL] Invoke() failed."); return;
  }

  // 5. Decode 2-class softmax output
  float prob_bird = (output->data.int8[1] - output->params.zero_point)
                    * output->params.scale;
  float prob_none = (output->data.int8[0] - output->params.zero_point)
                    * output->params.scale;

  bool detected = (prob_bird > prob_none);
  digitalWrite(LED_BUILTIN, detected ? LOW : HIGH);

  Serial.print(detected ? "BIRD   " : "silent ");
  Serial.print("| P(bird)="); Serial.print(prob_bird, 4);
  Serial.print(" P(none)="); Serial.println(prob_none, 4);
}
```

### 3.3 How to Run

1. Open `src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino` in Arduino IDE.
2. Confirm `BAD_tiny_model.h` is in the **same folder** as the `.ino` file.
3. **Tools → Board → Arduino Portenta H7 (M7 core)**
4. **Upload** → open **Serial Monitor** at **115200 baud**.

### 3.4 Expected Output

```
=== DrongoNet BAD CNN Smoke Test ===
[OK]  Model schema version matches.
[OK]  Input tensor : 1536 B
[OK]  Output tensor: 2 B
[OK]  Arena used   : 32768 B
[READY] Inference engine ready. Listening...

silent | P(bird)=0.0234 P(none)=0.9766
silent | P(bird)=0.0312 P(none)=0.9688
BIRD   | P(bird)=0.8741 P(none)=0.1259
BIRD   | P(bird)=0.9102 P(none)=0.0898
silent | P(bird)=0.1021 P(none)=0.8979
```

### 3.5 Pass / Fail Criteria

| Check | Pass Condition |
|---|---|
| Schema match | `[OK] Model schema version matches.` |
| AllocateTensors | No `[FAIL]` message, no hang |
| Arena used | Printed value < 150,000 B |
| Loop output | Alternates between `BIRD` and `silent` depending on mic input |
| P(bird) + P(none) | Sum ≈ 1.0 (valid softmax) |

---

## 4. Integration — Full CMSIS-DSP + DrongoNet Pipeline

### 4.1 Production Sketch Reference

The full production implementation with hardware-accelerated FFT is:
- **DrongoNet_Micro.ino** — drongonet-micro (16-mel, folded model)
- **DrongoNet_Nano.ino** — drongonet-nano (16-mel, explicit quantize/dequantize nodes)
- **DrongoNet_Edge.ino** — drongonet-nano (80-mel, explicit quantize/dequantize nodes)

> **Source files:**  
> [`src/DrongoNet/DrongoNet_Micro/Drongonet.ino`](../src/DrongoNet/DrongoNet_Micro/Drongonet.ino)  
> [`src/DrongoNet/DrongoNet_Nano/ DrongoNet_Nano.ino`](../src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino)
> [`src/DrongoNet/DrongoNet_Edge/DrongoNet_Edge.ino`](../src/DrongoNet/DrongoNet_Edge/DrongoNet_Edge.ino)

### 4.2 Op Resolver — Micro vs Nano

**drongonet-micro** (folded — no explicit QUANTIZE nodes):

```cpp
// Folded model: BN is fused into Conv2D kernels.
// 6 unique op types — NO QUANTIZE or DEQUANTIZE needed.
static tflite::MicroMutableOpResolver<6> resolver;
resolver.AddMul();             // opcode 18 v2 — FrequencyEmphasis
resolver.AddConv2D();          // opcode  3 v3 — Conv2D × 2
resolver.AddMaxPool2D();       // opcode 17 v2
resolver.AddMean();            // opcode 40 v2 — GlobalAveragePooling2D
resolver.AddFullyConnected();  // opcode  9 v4
resolver.AddSoftmax();         // opcode 25 v2
```

**drongonet-nano** (non-folded — explicit boundary nodes):

```cpp
// Non-folded model: QUANTIZE and DEQUANTIZE nodes are explicit.
// AddDequantize() is REQUIRED — omitting it causes a HardFault.
static tflite::MicroMutableOpResolver<8> resolver;
resolver.AddQuantize();        // opcode 114 v1 — REQUIRED
resolver.AddMul();             // opcode  18 v2
resolver.AddConv2D();          // opcode   3 v3
resolver.AddMaxPool2D();       // opcode  17 v2
resolver.AddMean();            // opcode  40 v2
resolver.AddFullyConnected();  // opcode   9 v4
resolver.AddSoftmax();         // opcode  25 v2
resolver.AddDequantize();      // opcode   6 v2 — REQUIRED
```

> **Why not AllOpsResolver for DrongoNet?** `AllOpsResolver` links every CMSIS-NN kernel (~70 ops) into the binary, adding ~150–250 KB of Flash. `MicroMutableOpResolver` registers only the ops the model actually uses, saving Flash for the larger MynaNet model in ARGUS.

### 4.3 CMSIS-DSP FFT Mel Spectrogram

This is the most critical component — the on-device mel must exactly match the Python/librosa pipeline used during model training.

```cpp
// ── DTCM placement — zero wait-state FFT working buffers ─────────────
// arm_rfft_fast_f32 inner loops access these arrays repeatedly per frame.
// DTCM eliminates AXI SRAM cache-miss stalls: ~3–8 ms saved per cycle.
// NOTE: DMA cannot access DTCM — captureAudio() uses CPU analogRead() (safe).
__attribute__((section(".dtcmram")))
static float hannWindow[1024];      // 4 KB — Hann window, N=1024
__attribute__((section(".dtcmram")))
static float fftBuf[1024];         // 4 KB — FFT in/out (in-place)
__attribute__((section(".dtcmram")))
static float powerSpectrum[513];   // 2 KB — |X[k]|² for k=0..512

static arm_rfft_fast_instance_f32 fftInstance;

// ── initMelFFT(): call once in setup() ───────────────────────────────
void initMelFFT() {
  // Hann window: w[i] = 0.5 × (1 − cos(2πi / N))
  for (int i = 0; i < 1024; i++)
    hannWindow[i] = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / 1024.0f));

  // Initialise CMSIS Real FFT (N=1024)
  arm_rfft_fast_init_f32(&fftInstance, 1024);

  // Pre-bake triangular filterbank weights (no runtime division in hot path)
  int offset = 0;
  for (int m = 0; m < 16 /*MEL_N_MELS*/; m++) {
    int s = melFilters[m].startBin;
    int p = melFilters[m].peakBin;
    int e = melFilters[m].endBin;
    int width = e - s;
    melFiltersFlat[m] = { s, width, offset };
    float rise_inv = (p > s) ? 1.0f / (float)(p - s) : 0.0f;
    float fall_inv = (e > p) ? 1.0f / (float)(e - p) : 0.0f;
    for (int k = s; k < e; k++) {
      float w = (k < p)  ? (float)(k-s) * rise_inv :
                (k == p) ? 1.0f :
                           (float)(e-k) * fall_inv;
      melWeightFlat[offset + (k - s)] = w;
    }
    offset += width;
  }
}
```

### 4.4 Full Mel Computation Per Frame

```cpp
// ── computeMelSpectrogram(): called every inference cycle ────────────
// Reproduces librosa.feature.melspectrogram(
//   y=audio, sr=16000, n_fft=1024, hop_length=256,
//   n_mels=16, fmin=100, fmax=8000, center=False)
void computeMelSpectrogram(int16_t *audio, int audioLen, float *melOut) {
  const float INV_32768 = 1.0f / 32768.0f;

  for (int f = 0; f < 184 /*TIME_FRAMES*/; f++) {
    int frameStart   = f * 256 /*MEL_HOP_LENGTH*/;
    int validSamples = audioLen - frameStart;
    if (validSamples > 1024) validSamples = 1024;

    // OPT-A: Fused int16→float + Hann window (single write pass)
    const int16_t *src = &audio[frameStart];
    for (int i = 0; i < validSamples; i++)
      fftBuf[i] = (float)src[i] * (hannWindow[i] * INV_32768);
    if (validSamples < 1024)
      memset(&fftBuf[validSamples], 0, (1024 - validSamples) * sizeof(float));

    // Real FFT — hardware-accelerated (Cortex-M7 FPU + SIMD)
    arm_rfft_fast_f32(&fftInstance, fftBuf, fftBuf, 0);

    // Power spectrum: |X[k]|² for k = 0..512
    powerSpectrum[0]   = fftBuf[0] * fftBuf[0];          // DC
    powerSpectrum[512] = fftBuf[1] * fftBuf[1];          // Nyquist
    arm_cmplx_mag_squared_f32(&fftBuf[2], &powerSpectrum[1], 511);

    // OPT-B: 4-way unrolled filterbank dot-product → dB
    float *melRow = &melOut[f * 16];
    for (int m = 0; m < 16; m++) {
      const float *P = &powerSpectrum[melFiltersFlat[m].startBin];
      const float *W = &melWeightFlat[melFiltersFlat[m].offset];
      int          w =  melFiltersFlat[m].width;
      float acc0=0,acc1=0,acc2=0,acc3=0; int k=0;
      for (; k <= w-4; k+=4) {
        acc0+=P[k]*W[k]; acc1+=P[k+1]*W[k+1];
        acc2+=P[k+2]*W[k+2]; acc3+=P[k+3]*W[k+3];
      }
      float acc = (acc0+acc1)+(acc2+acc3);
      for (; k < w; k++) acc += P[k]*W[k];
      melRow[m] = 10.0f * fast_log10f(acc + 1e-10f);
    }
  }

  // OPT-C: SIMD global min-max normalisation → [0, 1]
  float gMin, gMax; uint32_t idx;
  arm_min_f32(melOut, 16*184, &gMin, &idx);
  arm_max_f32(melOut, 16*184, &gMax, &idx);
  float range = (gMax - gMin < 1e-6f) ? 1e-6f : gMax - gMin;
  arm_offset_f32(melOut, -gMin, melOut, 16*184);
  arm_scale_f32(melOut, 1.0f/range, melOut, 16*184);
  for (int i = 0; i < 16*184; i++) {   // scalar clamp for safety
    if (melOut[i] < 0.0f) melOut[i] = 0.0f;
    if (melOut[i] > 1.0f) melOut[i] = 1.0f;
  }
}
```

### 4.5 INT8 Quantisation Into Input Tensor

```cpp
// Quantise float mel [0,1] → INT8 input tensor
// Formula: int8 = clamp(round(f / scale + zero_point), -128, 127)
for (int i = 0; i < INPUT_SIZE; i++) {
  float scaled = melFeatures[i] / input->params.scale
                 + input->params.zero_point;
  if (scaled >  127.0f) scaled =  127.0f;
  if (scaled < -128.0f) scaled = -128.0f;
  input->data.int8[i] = (int8_t)scaled;
}
```

### 4.6 Setup — Complete Initialisation Sequence

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
  DWT_ENABLE();   // Enable cycle counter for µs timing

  // Optional: INA219 power monitor
  Wire.begin();
  ina219_ok = ina219.begin();

  // Load model
  model = tflite::GetModel(drongonet_micro);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("[ERR] Schema mismatch."); while (1) { }
  }

  // Build interpreter with lean op resolver
  static tflite::MicroMutableOpResolver<6> resolver;
  resolver.AddMul(); resolver.AddConv2D(); resolver.AddMaxPool2D();
  resolver.AddMean(); resolver.AddFullyConnected(); resolver.AddSoftmax();

  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("[ERR] AllocateTensors() failed."); while (1) { }
  }
  input  = interpreter->input(0);
  output = interpreter->output(0);

  // Print memory summary
  printTFLMMemory();
  printFirmwareRAM();

  // Initialise CMSIS-DSP FFT + Hann window + filterbank
  initMelFFT();
  Serial.println("[OK] DrongoNet inference engine ready.");
}
```

---

## 5. Memory Architecture

### 5.1 Memory Layout

| Region | Contents | Size |
|---|---|---|
| DTCM (128 KB, zero wait) | `hannWindow[1024]`, `fftBuf[1024]`, `powerSpectrum[513]` | ~10 KB |
| AXI SRAM (512 KB, cached) | `audioBuffer[48000]`, `tensor_arena[150 KB]`, `melFeatures[2944]`, `melWeightFlat[931]` | ~265 KB |
| Flash (1 MB) | `drongonet_micro[]`, code, AllOpsResolver | ~300–500 KB |

### 5.2 Arena Sizing

The tensor arena must be ≥ `arena_used_bytes()`. For drongonet-micro the measured usage is 24–40 KB. The minimum recommended size is:

```
kTensorArenaSize_recommended = arena_used × 1.12   // +12% safety margin
```

For DrongoNet: `40 KB × 1.12 ≈ 45 KB`. Production sketches use **64–150 KB** as a generous development ceiling.

---

## 6. Output Decoding and Temporal Smoothing

### 6.1 Dequantise INT8 Output

```cpp
// Decode 2-class softmax output (INT8 → float probability)
float prob_bird = (output->data.int8[1] - output->params.zero_point)
                  * output->params.scale;
float prob_none = (output->data.int8[0] - output->params.zero_point)
                  * output->params.scale;
// Note: prob_bird + prob_none ≈ 1.0 (softmax invariant)
```

### 6.2 Three-Window Temporal Smoothing

A single noisy frame cannot trigger a bird detection. The 3-window ring buffer requires consistent scores across ~9 seconds of audio before reporting a bird:

```cpp
// 3-window temporal smoothing ring buffer
static float scoreHistory[3] = {0.0f, 0.0f, 0.0f};
static int   scoreHistIdx     = 0;

scoreHistory[scoreHistIdx % 3] = prob_bird;
scoreHistIdx++;
float avgScore = (scoreHistory[0] + scoreHistory[1] + scoreHistory[2]) / 3.0f;

bool birdDetected = (avgScore > 0.60f);  // BAD_AVG_THRESHOLD
digitalWrite(LED_BUILTIN, birdDetected ? LOW : HIGH);

Serial.print(birdDetected ? "BIRD  " : "quiet ");
Serial.print("| raw="); Serial.print(prob_bird, 4);
Serial.print(" avg="); Serial.print(avgScore, 4);
Serial.print(" rms="); Serial.println(rms, 0);
```

### 6.3 Threshold Reference

| Threshold | Default | Purpose |
|---|---|---|
| `RMS_THRESHOLD` | 500.0 | Skip inference on silent windows |
| `BAD_RAW_THRESHOLD` | 0.35 | Per-frame bird score (reference only) |
| `BAD_AVG_THRESHOLD` | 0.60 | Smoothed decision gate |

---

## 7. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| `AllocateTensors() failed` | Arena too small | Increase `kTensorArenaSize` to 200 * 1024 |
| `Schema mismatch` | Old TFLite Micro library | Update `Arduino_TensorFlowLite` library |
| `Didn't find op for builtin opcode N` | Op missing from resolver | Cross-check op list against model flatbuffer |
| P(bird) always ~0.5 | Model not loaded | Confirm model header `.h` in same folder |
| P(bird) always 0 or 1 | Wrong zero_point or scale | Print `input->params.scale` and `zero_point` at startup |
| HardFault / reset after Invoke() | AddDequantize() missing (nano only) | Add `resolver.AddDequantize()` for nano model |
| Mel always identical | ADC not sampling | Confirm mic pin A0/A1 matches wiring and `MIC_PIN` constant |
| `abs` macro error | Arduino.h conflict | Add `#ifdef abs` / `#undef abs` before `<cmath>` |

---

*Previous →* **[Guide 02 — INA219 Current Sensor](Guide_02_INA219_Current_Sensor.md)**  
*Next →* **[Guide 04 — MynaNet Species Classifier CNN](Guide_04_MynaNet_Species_CNN.md)**
