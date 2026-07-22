# Guide 04 — MynaNet Species Classifier CNN
## Arduino Portenta H7 Bird Recognition System

> **Model**: MynaNet-1j DS-CNN (Depthwise-Separable CNN, INT8 Quantised)  
> **Task**: 12-Class Malaysian Garden Bird Species Classification  
> **Input**: 64-bin × 300-frame Mel Spectrogram, sampled at 16 kHz  
> **Hardware**: Arduino Portenta H7 (STM32H747XI, Cortex-M7 @ 480 MHz)  
> **Accuracy**: 94.91% top-1 across 12 species  

---

## Table of Contents

1. [Model Overview](#1-model-overview)
2. [Library Installation](#2-library-installation)
3. [Smoke Test — MynaNet Inference from a Pre-Computed Header](#3-smoke-test--mynanet-inference-from-a-pre-computed-header)
4. [Integration — Full DS-CNN Pipeline in CascadeArgus](#4-integration--full-ds-cnn-pipeline-in-cascadeargus)
5. [SDRAM Memory Architecture](#5-sdram-memory-architecture)
6. [Output Decoding — Top-K Species Extraction](#6-output-decoding--top-k-species-extraction)
7. [Generating Input Headers on Desktop (audio_feeding.py)](#7-generating-input-headers-on-desktop)
8. [Troubleshooting](#8-troubleshooting)

---

## 1. Model Overview

### 1.1 Architecture — DS-CNN with 4 Depthwise-Separable Blocks

MynaNet-1j is a MobileNet-style Depthwise-Separable CNN optimised for INT8 inference on the Cortex-M7. Depthwise-separable convolutions reduce multiply-accumulate count by ~8–9× compared to standard Conv2D with the same filter counts.

| Layer | Output Shape | Notes |
|---|---|---|
| Input | [1, 64, 300, 1] | 64 mel bins × 300 time frames, INT8 |
| Stem Conv2D (32ch, 3×3) + BN + ReLU6 | [1, 64, 300, 32] | Peak activation: ~600 KB |
| DS Block 1 (DW 3×3 + PW 1×1 → 16ch + MaxPool 2×2) | [1, 32, 150, 16] | |
| DS Block 2 (expand 96ch + DW + PW → 24ch + MaxPool) | [1, 16, 75, 24] | |
| DS Block 3 (expand 144ch + DW + PW → 48ch + MaxPool) | [1, 8, 37, 48] | |
| DS Block 4 (expand 288ch + DW + PW → 96ch + MaxPool) | [1, 4, 18, 96] | |
| GlobalAveragePooling2D (MEAN) | [1, 96] | |
| Reshape | [1, 96] | |
| Dense (128, ReLU6) | [1, 128] | |
| Dense (12, Softmax) | [1, 12] | Output: 12 species probabilities |

**Model size: ~267 KB in Flash (Mynanet_1j.h)**

### 1.2 Mel Spectrogram Parameters

MynaNet uses **different** FFT parameters than DrongoNet. Both models must be computed from the same raw audio buffer but with independent FFT calls:

| Parameter | Value | Notes |
|---|---|---|
| Sample rate | 16,000 Hz | |
| Audio duration | 3 seconds | 48,000 samples |
| n_fft | **512** | (vs 1024 for DrongoNet) |
| hop_length | **160** | 10 ms hop (vs 256 for DrongoNet) |
| n_mels | **64** | (vs 16 for DrongoNet) |
| fmin | 0 Hz | Full spectrum |
| fmax | 8,000 Hz | Nyquist limit |
| Time frames | **300** | ⌊(48000 − 512) / 160⌋ + 1 → zero-padded to 300 |
| Input tensor | 64 × 300 = **19,200 elements** | |

### 1.3 Registered TFLite Ops

MynaNet's DS-CNN graph uses exactly 7 operator types. Using `MicroMutableOpResolver<7>` instead of `AllOpsResolver` saves 150–250 KB of Flash:

| Op | Usage |
|---|---|
| `CONV_2D` | Stem (×1) + 4× pointwise PW (×4) = 5 invocations |
| `DEPTHWISE_CONV_2D` | 4× depthwise spatial filters |
| `MAX_POOL_2D` | 4× 2×2 downsampling |
| `MEAN` | GlobalAveragePooling2D |
| `FULLY_CONNECTED` | Dense(128) + Dense(12) |
| `SOFTMAX` | Final output |
| `RESHAPE` | [B,1,1,C] → [B,C] |

### 1.4 Species Labels (12 Classes)

| Index | Species | Call Character |
|---|---|---|
| 0 | Coppersmith Barbet | Metallic "tuk-tuk-tuk" |
| 1 | Spotted Dove | Low-frequency cooing |
| 2 | Collared Kingfisher | Piercing "kee-kee-kee" |
| 3 | Asian Koel | Ascending "ko-el" |
| 4 | White-breasted Waterhen | Harsh "kwak-kwak" |
| 5 | Common Iora | Melodious whistling |
| 6 | Large-tailed Nightjar | Churring nocturnal |
| 7 | Yellow-vented Bulbul | Cheerful warbling |
| 8 | Pied Fantail | Sharp reactive calls |
| 9 | Olive-backed Sunbird | High "cheet-cheet" |
| 10 | Common Tailorbird | Loud "towit-towit" |
| 11 | White-throated Kingfisher | Chuckling "chake-ake" |

---

## 2. Library Installation

### 2.1 TensorFlow Lite Micro

**Install via Library Manager:**

1. **Tools → Manage Libraries…**
2. Search: `TensorFlowLite`
3. Install: **Arduino_TensorFlowLite** ≥ 2.4.0

```cpp
#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
```

### 2.2 CMSIS-DSP (Bundled — No Install)

MynaNet's mel spectrogram uses `arm_rfft_fast_f32` with **N=512** (different from DrongoNet's N=1024):

```cpp
#include <arm_math.h>   // Bundled with Arduino Mbed OS Portenta board package
```

### 2.3 SDRAM Library (Bundled — No Install)

MynaNet's 2 MB tensor arena exceeds on-chip AXI SRAM. It must be allocated from external SDRAM:

```cpp
#include <SDRAM.h>   // Bundled with Arduino Mbed OS Portenta board package
```

### 2.4 Flash Split — REQUIRED for CascadeArgus

CascadeArgus loads both DrongoNet (~5 KB) and MynaNet (~267 KB) simultaneously. The combined binary exceeds the default 1 MB M7 Flash partition.

**Required setting:**
```
Tools → Flash Split → 1.5 MB M7 + 0.5 MB M4
```

Without this, the linker reports:
```
region 'FLASH' overflowed by N bytes
```

---

## 3. Smoke Test — MynaNet Inference from a Pre-Computed Header

### 3.1 Purpose

Validate that the MynaNet model loads, the SDRAM arena allocates, and inference produces correct top-3 species predictions — without requiring a live microphone or CMSIS-DSP mel pipeline. This uses a pre-computed INT8 mel spectrogram embedded as a C header file (`Spotted_Dove_mel.h`).

This is the validation approach used in [`src/MynaNet/MynaNet.ino`](../src/MynaNet/MynaNet.ino).

### 3.2 Smoke Test Sketch

```cpp
// ================================================================
// Smoke Test: MynaNet Species Classifier CNN
// Uses a pre-computed INT8 mel header (Spotted_Dove_mel.h)
// as model input — no microphone or FFT required.
//
// Expected output:
//   1. Spotted Dove       → 82.34 %
//   2. Asian Koel         → 10.12 %
//   3. Common Iora        →  3.56 %
//   Inference latency: ~45–120 ms
// ================================================================

#include "Arduino.h"
#ifdef abs
#undef abs
#endif

#include "TensorFlowLite.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"

#include <SDRAM.h>

#include "Mynanet_1j.h"         // Model flat-buffer (~267 KB in Flash)
#include "Spotted_Dove_mel.h"   // Pre-computed INT8 mel: const int8_t[19200]

// ── Compile-time shape assertion ─────────────────────────────────
// Catches mismatches between mel header and model input at build time.
static_assert(sizeof(Spotted_Dove_mel) == 64 * 300 * 1,
    "Spotted_Dove_mel size does not match model input [1,64,300,1]");

// ── Species labels ────────────────────────────────────────────────
const int NUM_CLASSES = 12;
const char *SPECIES_NAMES[12] = {
  "Coppersmith Barbet", "Spotted Dove", "Collared Kingfisher",
  "Asian Koel", "White-breasted Waterhen", "Common Iora",
  "Large-tailed Nightjar", "Yellow-vented Bulbul", "Pied Fantail",
  "Olive-backed Sunbird", "Common Tailorbird", "White-throated Kingfisher"
};

// ── TFLite Micro runtime ──────────────────────────────────────────
namespace {
  tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter      *error_reporter = &micro_error_reporter;
  const tflite::Model        *model       = nullptr;
  tflite::MicroInterpreter   *interpreter = nullptr;
  TfLiteTensor               *input  = nullptr;
  TfLiteTensor               *output = nullptr;

  // 2 MB arena in SDRAM — DS-CNN peak activation is ~600 KB at stem output
  constexpr int kTensorArenaSize = 2 * 1024 * 1024;
  uint8_t *tensor_arena = nullptr;

  static tflite::MicroMutableOpResolver<7> resolver;
}

// ── Top-K extraction ──────────────────────────────────────────────
// Dequantise INT8 output and return top-k (idx, prob) pairs.
void topK(TfLiteTensor *out, int numClasses, int k,
          int *topIdx, float *topProb) {
  for (int i = 0; i < k; i++) { topIdx[i] = -1; topProb[i] = -1.0f; }

  for (int i = 0; i < numClasses; i++) {
    float p = (out->data.int8[i] - out->params.zero_point) * out->params.scale;
    if (p > topProb[k-1]) {
      topProb[k-1] = p; topIdx[k-1] = i;
      // Insertion sort: bubble up
      for (int j = k-1; j > 0 && topProb[j] > topProb[j-1]; j--) {
        float tp = topProb[j]; topProb[j] = topProb[j-1]; topProb[j-1] = tp;
        int   ti = topIdx[j];  topIdx[j]  = topIdx[j-1];  topIdx[j-1]  = ti;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  delay(2000);

  Serial.println("=== MynaNet Species Classifier Smoke Test ===");

  // 1. Initialise SDRAM
  SDRAM.begin();
  uint8_t *raw = (uint8_t *)SDRAM.malloc(kTensorArenaSize + 15);
  if (!raw) {
    Serial.println("[FAIL] SDRAM.malloc() failed."); while (1) { }
  }
  tensor_arena = (uint8_t *)(((uintptr_t)raw + 15u) & ~15u);  // 16-byte align
  Serial.print("[OK]  SDRAM arena at 0x"); Serial.println((uint32_t)tensor_arena, HEX);

  // 2. Load model
  model = tflite::GetModel(MynaNet_1j);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("[FAIL] Schema mismatch."); while (1) { }
  }
  Serial.println("[OK]  Model schema version matches.");

  // 3. Register ops — exact 7 ops used by MynaNet DS-CNN
  resolver.AddConv2D();
  resolver.AddDepthwiseConv2D();
  resolver.AddMaxPool2D();
  resolver.AddMean();
  resolver.AddFullyConnected();
  resolver.AddSoftmax();
  resolver.AddReshape();

  // 4. Build interpreter
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  // 5. Allocate tensors
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("[FAIL] AllocateTensors() — SDRAM arena too small.");
    while (1) { }
  }
  input  = interpreter->input(0);
  output = interpreter->output(0);

  Serial.print("[OK]  Input tensor : "); Serial.print(input->bytes);
  Serial.println(" B  (INT8 mel [1,64,300,1])");
  Serial.print("[OK]  Output tensor: "); Serial.print(output->bytes);
  Serial.println(" B  (INT8 softmax [1,12])");
  Serial.print("[OK]  Arena used   : "); Serial.print(interpreter->arena_used_bytes());
  Serial.println(" B");
  Serial.println("[READY] MynaNet inference engine ready.\n");
}

void loop() {
  unsigned long t0 = millis();

  // 1. Copy pre-computed mel header → input tensor (Flash → SDRAM)
  memcpy(input->data.int8, Spotted_Dove_mel, input->bytes);

  // 2. D-cache flush — REQUIRED when CPU writes SDRAM and CMSIS-NN reads it
  // Without this: dirty cache lines may not yet be visible to CMSIS-NN kernels.
  SCB_CleanInvalidateDCache();

  // 3. Run DS-CNN inference
  if (interpreter->Invoke() != kTfLiteOk) {
    Serial.println("[FAIL] Invoke() failed."); delay(1000); return;
  }

  unsigned long latency = millis() - t0;

  // 4. Extract top-3 species
  int   topIdx[3];
  float topProb[3];
  topK(output, NUM_CLASSES, 3, topIdx, topProb);

  // 5. Print results
  Serial.println("╔══════════════════════════════╗");
  Serial.println("║   MynaNet — Top 3 Prediction ║");
  Serial.println("╚══════════════════════════════╝");
  for (int k = 0; k < 3; k++) {
    Serial.print(" "); Serial.print(k + 1); Serial.print(". ");
    Serial.print(SPECIES_NAMES[topIdx[k]]);
    Serial.print("  →  ");
    Serial.print(topProb[k] * 100.0f, 2); Serial.println(" %");
  }
  Serial.print("  Inference latency : "); Serial.print(latency);
  Serial.println(" ms");
  Serial.println();

  delay(3000);  // Re-run every 3 seconds
}
```

### 3.3 Required Files

Place all of the following in the **same folder** as your `.ino` file:

```
MynaNet_smoke_test/
├── MynaNet_smoke_test.ino
├── Mynanet_1j.h              ← DS-CNN model (~267 KB)
└── Spotted_Dove_mel.h        ← Pre-computed INT8 mel (19,200 bytes)
```

Both headers are available in [`src/MynaNet/`](../src/MynaNet/).

### 3.4 How to Run

1. Open the sketch in Arduino IDE.
2. **Tools → Flash Split → 1.5 MB M7 + 0.5 MB M4**
3. **Tools → Board → Arduino Portenta H7 (M7 core)**
4. **Upload** → open **Serial Monitor** at **115200 baud**.

### 3.5 Expected Output

```
=== MynaNet Species Classifier Smoke Test ===
[OK]  SDRAM arena at 0x60000010
[OK]  Model schema version matches.
[OK]  Input tensor : 19200 B  (INT8 mel [1,64,300,1])
[OK]  Output tensor: 12 B  (INT8 softmax [1,12])
[OK]  Arena used   : 679936 B
[READY] MynaNet inference engine ready.

╔══════════════════════════════╗
║   MynaNet — Top 3 Prediction ║
╚══════════════════════════════╝
 1. Spotted Dove          →  82.34 %
 2. Asian Koel            →  10.12 %
 3. Common Iora           →   3.56 %
  Inference latency : 47 ms
```

### 3.6 Pass / Fail Criteria

| Check | Pass Condition |
|---|---|
| SDRAM allocation | `SDRAM arena at 0x6XXXXXXX` (external SDRAM address space) |
| AllocateTensors | No `[FAIL]` message |
| Arena used | Printed value < 2,097,152 B (2 MB) |
| Top-1 species | Spotted Dove for `Spotted_Dove_mel.h` input |
| Inference latency | 40–150 ms |
| No HardFault | Stable output repeated every 3 seconds |

---

## 4. Integration — Full DS-CNN Pipeline in CascadeArgus

### 4.1 System Position

In CascadeArgus, MynaNet runs **only when DrongoNet confirms bird activity** (cascade gate). MynaNet is never invoked for silent or background-noise windows — saving ~100 ms + ~80 mJ per cycle.

> **Source file:** [`src/CascadeArgus/CascadeArgus.ino`](../src/CascadeArgus/CascadeArgus.ino)

### 4.2 MynaNet Mel Spectrogram (N=512, 64 bins)

MynaNet requires its own independent FFT instance — **different from DrongoNet's N=1024 instance**. Both are computed from the **same** `audioBuffer` (no re-capture):

```cpp
// ── DTCM placement — MynaNet FFT working buffers ─────────────────────
__attribute__((section(".dtcmram")))
static float hannWindow512[512];    // Hann window, N=512 (2 KB)
__attribute__((section(".dtcmram")))
static float fftBuf512[512];       // In-place FFT buffer (2 KB)
__attribute__((section(".dtcmram")))
static float powerSpectrum512[257]; // |X[k]|² for k=0..256 (1 KB)

static arm_rfft_fast_instance_f32 fftInstanceMyna;

void initMynaFFT() {
  // N=512 Hann window
  for (int i = 0; i < 512; i++)
    hannWindow512[i] = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / 512.0f));

  // CMSIS Real FFT — N=512
  arm_rfft_fast_init_f32(&fftInstanceMyna, 512);

  // Pre-bake 64-bin triangular filterbank weights
  // (uses mynanet_mel_tables_64.h — loaded from mel_tables.h in CascadeArgus)
  int offset = 0;
  for (int m = 0; m < 64; m++) {
    // ... (same filterbank baking logic as DrongoNet, but with 64 filters)
  }
}
```

### 4.3 MynaNet Mel Computation

```cpp
// 64-bin mel for MynaNet — n_fft=512, hop=160, 300 frames
// All parameters match audio_feeding.py training script exactly.
void computeMynanetMel(int16_t *audio, int audioLen, float *melOut) {
  const float INV_32768 = 1.0f / 32768.0f;
  const int N_FFT       = 512;
  const int HOP         = 160;
  const int N_FRAMES    = 300;
  const int N_MELS      = 64;

  for (int f = 0; f < N_FRAMES; f++) {
    int frameStart   = f * HOP;
    int validSamples = audioLen - frameStart;
    if (validSamples > N_FFT) validSamples = N_FFT;

    // Fused int16→float + Hann window (single pass)
    const int16_t *src = &audio[frameStart];
    for (int i = 0; i < validSamples; i++)
      fftBuf512[i] = (float)src[i] * (hannWindow512[i] * INV_32768);
    if (validSamples < N_FFT)
      memset(&fftBuf512[validSamples], 0, (N_FFT - validSamples) * sizeof(float));

    // Real FFT (N=512)
    arm_rfft_fast_f32(&fftInstanceMyna, fftBuf512, fftBuf512, 0);

    // Power spectrum
    powerSpectrum512[0]       = fftBuf512[0] * fftBuf512[0];  // DC
    powerSpectrum512[N_FFT/2] = fftBuf512[1] * fftBuf512[1];  // Nyquist
    arm_cmplx_mag_squared_f32(&fftBuf512[2], &powerSpectrum512[1], N_FFT/2 - 1);

    // 64-bin filterbank → dB
    float *melRow = &melOut[f * N_MELS];
    for (int m = 0; m < N_MELS; m++) {
      const float *P = &powerSpectrum512[mynaFiltersFlat[m].startBin];
      const float *W = &mynaWeightFlat[mynaFiltersFlat[m].offset];
      int          w =  mynaFiltersFlat[m].width;
      float acc = 0.0f;
      for (int k = 0; k < w; k++) acc += P[k] * W[k];
      melRow[m] = 10.0f * fast_log10f(acc + 1e-10f);
    }
  }

  // Global normalisation → [0, 1]
  float gMin, gMax; uint32_t idx;
  arm_min_f32(melOut, N_MELS * N_FRAMES, &gMin, &idx);
  arm_max_f32(melOut, N_MELS * N_FRAMES, &gMax, &idx);
  float range = (gMax - gMin < 1e-6f) ? 1e-6f : gMax - gMin;
  arm_offset_f32(melOut, -gMin, melOut, N_MELS * N_FRAMES);
  arm_scale_f32(melOut, 1.0f / range, melOut, N_MELS * N_FRAMES);
  for (int i = 0; i < N_MELS * N_FRAMES; i++) {
    if (melOut[i] < 0.0f) melOut[i] = 0.0f;
    if (melOut[i] > 1.0f) melOut[i] = 1.0f;
  }
}
```

### 4.4 MynaNet Inference in CascadeArgus Loop

```cpp
// ── Runs only when DrongoNet avgScore > BAD_AVG_THRESHOLD ────────────
void runMynaNet() {
  Serial.println("[CASCADE] Bird confirmed — running species ID...");

  // 1. Compute MynaNet mel from the same audioBuffer (no re-capture)
  computeMynanetMel(audioBuffer, BUFFER_SIZE, mynanetMelFeatures);

  // 2. Quantise float mel → INT8 input tensor
  for (int i = 0; i < 64 * 300; i++) {
    float scaled = mynanetMelFeatures[i] / myna_input->params.scale
                   + myna_input->params.zero_point;
    if (scaled >  127.0f) scaled =  127.0f;
    if (scaled < -128.0f) scaled = -128.0f;
    myna_input->data.int8[i] = (int8_t)scaled;
  }

  // 3. D-cache flush — CPU wrote to SDRAM; CMSIS-NN will read from SDRAM
  //    Without this: stale cache lines may cause incorrect inference.
  SCB_CleanInvalidateDCache();

  // 4. Run DS-CNN inference
  unsigned long inferStart = millis();
  if (myna_interp->Invoke() != kTfLiteOk) {
    Serial.println("[ERR] MynaNet Invoke() failed."); return;
  }
  unsigned long inferMs = millis() - inferStart;

  // 5. Extract top-3 species
  int   topIdx[3];
  float topProb[3];
  topK(myna_output, 12, 3, topIdx, topProb);

  // 6. Print results
  Serial.println("\n╔══════════════════════════════╗");
  Serial.println("║   MynaNet — Species Detected  ║");
  Serial.println("╚══════════════════════════════╝");
  for (int k = 0; k < 3; k++) {
    Serial.print(" "); Serial.print(k + 1); Serial.print(". ");
    Serial.print(SPECIES_NAMES[topIdx[k]]);
    Serial.print("  →  ");
    Serial.print(topProb[k] * 100.0f, 2); Serial.println(" %");
  }
  Serial.print("  Inference : "); Serial.print(inferMs); Serial.println(" ms\n");
}
```

---

## 5. SDRAM Memory Architecture

### 5.1 Why SDRAM Is Required

MynaNet's DS-CNN peak activation is ~600 KB at the stem output ([1,64,300,32]). This exceeds the Portenta H7's on-chip AXI SRAM (512 KB) after subtracting the audio buffer (96 KB) and other globals. The 8 MB external SDRAM (at address `0x60000000`) is the only viable location.

### 5.2 SDRAM Allocation Pattern

```cpp
#include <SDRAM.h>

constexpr int kMynaArenaSize   = 2 * 1024 * 1024;  // 2 MB arena
constexpr int kMynaBufferElems = 64 * 300;          // 19,200 floats mel buffer

uint8_t *myna_arena       = nullptr;
float   *mynanetMelBuffer = nullptr;

void setup() {
  SDRAM.begin();   // MUST be called before SDRAM.malloc()

  // Allocate arena with 16-byte alignment (CMSIS-NN SIMD requirement)
  uint8_t *raw = (uint8_t *)SDRAM.malloc(kMynaArenaSize + 15);
  if (!raw) { Serial.println("[FATAL] SDRAM arena alloc failed."); while(1){} }
  myna_arena = (uint8_t *)(((uintptr_t)raw + 15u) & ~15u);

  // Allocate mel feature buffer (76,800 B = 75 KB)
  mynanetMelBuffer = (float *)SDRAM.malloc(kMynaBufferElems * sizeof(float));
  if (!mynanetMelBuffer) { Serial.println("[FATAL] SDRAM mel alloc failed."); while(1){} }
}
```

### 5.3 D-Cache Coherency — Critical Detail

The Cortex-M7 has a 16 KB D-cache that sits between the CPU and external SDRAM. When `memcpy()` or mel quantisation writes to SDRAM, the data may be buffered in cache and **not yet visible** to CMSIS-NN kernels that read SDRAM directly via the AXI bus.

**Always flush the D-cache before `Invoke()`:**

```cpp
// After CPU writes to input tensor (SDRAM), before CMSIS-NN reads it:
SCB_CleanInvalidateDCache();
interpreter->Invoke();   // Now safe — SDRAM data is coherent
```

**Without this flush:** inference runs on stale or zeroed data, producing random / always-zero output.

### 5.4 Memory Budget Summary

| Region | Contents | Size |
|---|---|---|
| AXI SRAM (512 KB) | `audioBuffer[48000]` + DrongoNet arena + mel features + globals | ~330 KB used |
| SDRAM (8 MB) | MynaNet arena + MynaNet mel buffer | ~2.07 MB used |
| DTCM (128 KB) | FFT working buffers (DrongoNet + MynaNet) | ~20 KB used |
| Flash (1.5 MB with 75/25 split) | DrongoNet model + MynaNet model + code | ~650–900 KB |

---

## 6. Output Decoding — Top-K Species Extraction

### 6.1 Dequantise INT8 → Float Probability

```cpp
// Output tensor: [1, 12], INT8, scale and zero_point from model
// float_prob = (int8_value - zero_point) × scale
for (int i = 0; i < 12; i++) {
  float p = (myna_output->data.int8[i] - myna_output->params.zero_point)
            * myna_output->params.scale;
  // p is now in [0.0, 1.0]; sum of all 12 ≈ 1.0 (softmax)
}
```

### 6.2 Top-K Extraction (Insertion Sort)

```cpp
// O(n × k) algorithm: n=12 classes, k=3 → max 36 comparisons
void topK(TfLiteTensor *out, int numClasses, int k,
          int *topIdx, float *topProb) {
  for (int i = 0; i < k; i++) { topIdx[i] = -1; topProb[i] = -1.0f; }

  for (int i = 0; i < numClasses; i++) {
    float p = (out->data.int8[i] - out->params.zero_point) * out->params.scale;
    if (p > topProb[k-1]) {
      topProb[k-1] = p; topIdx[k-1] = i;
      for (int j = k-1; j > 0 && topProb[j] > topProb[j-1]; j--) {
        // Swap adjacent elements to maintain descending order
        float tp = topProb[j]; topProb[j] = topProb[j-1]; topProb[j-1] = tp;
        int   ti = topIdx[j];  topIdx[j]  = topIdx[j-1];  topIdx[j-1]  = ti;
      }
    }
  }
}
```

### 6.3 Confidence Interpretation

| Top-1 Confidence | Interpretation | Action |
|---|---|---|
| > 80% | High-confidence identification | Report species name |
| 50%–80% | Moderate confidence | Report with "likely" qualifier |
| 30%–50% | Low confidence | Report as "possible" |
| < 30% | Very uncertain | Report as "unknown bird" |

---

## 7. Generating Input Headers on Desktop

For the smoke test, the mel header is generated by a Python script (`audio_feeding.py`) that replicates the exact librosa parameters used during MynaNet training.

### 7.1 Requirements

```bash
pip install librosa numpy soundfile
```

### 7.2 Mel Header Generation Script

```python
# audio_feeding.py — generates INT8 mel spectrogram C header for MynaNet smoke test
# Matches exactly: librosa.feature.melspectrogram(y, sr=16000, n_fft=512,
#                  hop_length=160, n_mels=64, fmin=0, fmax=8000, center=False)

import librosa
import numpy as np
import os

def generate_mel_header(wav_path, output_h_path, var_name="my_bird_mel"):
    y, sr = librosa.load(wav_path, sr=16000, mono=True, duration=3.0)
    if len(y) < sr * 3:
        y = np.pad(y, (0, sr * 3 - len(y)))  # zero-pad to exactly 3 seconds

    mel = librosa.feature.melspectrogram(
        y=y, sr=16000, n_fft=512, hop_length=160,
        n_mels=64, fmin=0, fmax=8000, center=False
    )
    mel_db = librosa.power_to_db(mel, ref=np.max)

    # Normalise to [0, 1]
    mel_norm = (mel_db - mel_db.min()) / (mel_db.max() - mel_db.min() + 1e-9)

    # Crop or pad time dimension to exactly 300 frames
    if mel_norm.shape[1] > 300:
        mel_norm = mel_norm[:, :300]
    elif mel_norm.shape[1] < 300:
        mel_norm = np.pad(mel_norm, ((0,0), (0, 300 - mel_norm.shape[1])))

    # Quantise float [0,1] → INT8 [-128, 127]
    # Maps: 0.0 → -128, 1.0 → 127
    mel_int8 = np.clip(np.round(mel_norm * 255.0) - 128, -128, 127).astype(np.int8)
    assert mel_int8.shape == (64, 300), f"Shape mismatch: {mel_int8.shape}"

    flat = mel_int8.flatten()   # Shape: [19200] — row-major (bins × frames)

    with open(output_h_path, 'w') as f:
        f.write(f'// Auto-generated by audio_feeding.py\n')
        f.write(f'// Source: {os.path.basename(wav_path)}\n')
        f.write(f'// Shape: [1, 64, 300, 1] INT8 — matches MynaNet input\n\n')
        f.write(f'#pragma once\n')
        f.write(f'#include <stdint.h>\n\n')
        f.write(f'const int8_t {var_name}[] = {{\n  ')
        hex_vals = [f'0x{(b & 0xFF):02X}' for b in flat]
        for i, v in enumerate(hex_vals):
            f.write(v)
            if i < len(hex_vals) - 1:
                f.write(', ')
            if (i + 1) % 16 == 0:
                f.write('\n  ')
        f.write('\n};\n')

    print(f"Generated: {output_h_path}  ({len(flat)} bytes)")

# Example usage:
generate_mel_header("Spotted_Dove_call.wav", "Spotted_Dove_mel.h", "Spotted_Dove_mel")
```

### 7.3 Using the Generated Header

1. Run the script on your desktop.
2. Copy the generated `.h` file into the sketch folder.
3. Upload the smoke test sketch to the Portenta H7.

---

## 8. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| `region 'FLASH' overflowed` | Flash split not set | **Tools → Flash Split → 1.5 MB M7 + 0.5 MB M4** |
| `SDRAM arena alloc failed` | SDRAM.begin() not called | Call `SDRAM.begin()` before `SDRAM.malloc()` |
| `AllocateTensors() failed` | Arena too small | Increase `kMynaArenaSize` to 3 * 1024 * 1024 |
| `Didn't find op for builtin opcode` | Op resolver incomplete | Add missing op (e.g., `resolver.AddReshape()`) |
| All species output 0% | D-cache not flushed | Add `SCB_CleanInvalidateDCache()` before `Invoke()` |
| Always same species (wrong species) | Header shape mismatch | Check `static_assert(sizeof(...) == 64*300*1)` |
| HardFault / crash | SDRAM not initialised | `SDRAM.begin()` before any SDRAM access |
| `Arena used` > `kMynaArenaSize` | Arena undersized | Print `arena_used_bytes()` and set `kMynaArenaSize = used × 1.2` |
| Inference latency > 200 ms | Op resolver has wrong ops | Confirm `MicroMutableOpResolver<7>` has all 7 correct ops |

---

*Previous →* **[Guide 03 — DrongoNet BAD CNN](Guide_03_DrongoNet_BAD_CNN.md)**  
*Back to Start →* **[Guide 01 — Microphone](Guide_01_Microphone.md)**
