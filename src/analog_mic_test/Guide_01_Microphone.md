# Guide 01 — Microphone (MAX4466 Analog Mic)
## Arduino Portenta H7 Bird Recognition System

> **Component**: MAX4466 Electret Microphone Amplifier → Analog Pin A0 / A1  
> **Role in System**: Captures 3-second, 16 kHz audio windows fed to the DSP mel-spectrogram pipeline  
> **Hardware**: Arduino Portenta H7 (STM32H747XI, Cortex-M7 @ 480 MHz)  

---

## Table of Contents

1. [Component Overview](#1-component-overview)
2. [Library Installation](#2-library-installation)
3. [Smoke Test — Verify Mic Wiring and ADC Response](#3-smoke-test--verify-mic-wiring-and-adc-response)
4. [Integration — Audio Capture in the Inference Pipeline](#4-integration--audio-capture-in-the-inference-pipeline)
5. [Tuning and Calibration](#5-tuning-and-calibration)
6. [Troubleshooting](#6-troubleshooting)

---

## 1. Component Overview

### 1.1 What the MAX4466 Does

The MAX4466 is a low-noise electret microphone amplifier IC with adjustable gain (trimmer potentiometer). It converts acoustic pressure waves into an analog voltage that the Portenta H7's 12-bit ADC can sample at 16 kHz.

| Parameter | Value |
|---|---|
| Supply voltage | 2.4–5.5 V (use **3.3 V** with Portenta H7) |
| Output voltage | Centred at VCC/2 ≈ 1.65 V (resting state) |
| ADC resolution | 12-bit → 0–4095 counts; midpoint ≈ 2048 |
| Gain range | 25–125× (adjustable via onboard trimmer) |
| Frequency response | 20 Hz – 20 kHz |

### 1.2 Wiring

| MAX4466 Pin | Portenta H7 Pin | Notes |
|---|---|---|
| VCC | 3.3 V | **Never use 5 V** — Portenta ADC is 3.3 V tolerant |
| GND | GND | Common ground |
| OUT | **A0-A7** | User could select any of **A0 - A7** analog pin to use for the MAX4466, but it must be consistent with the sketch. |

---

## 2. Library Installation

### 2.1 Required Board Package

The Portenta H7's analog ADC is accessed via the standard Arduino `analogRead()` API — **no external library is required** for basic audio capture. All necessary drivers are provided by the **Arduino Mbed OS Portenta Boards** board package.

**Installation (one-time, if not already done):**

1. Open **Arduino IDE → Tools → Board → Boards Manager**
2. Search: `Arduino Mbed OS Portenta Boards`
3. Install the latest version (≥ 4.x)

### 2.2 ADC Configuration (Built-In)

The Portenta H7 ADC defaults to **12-bit resolution** (0–4095). No configuration call is needed in the sketch — `analogRead()` returns values in this range automatically.

```cpp
// No library imports needed for the microphone itself.
// The standard Arduino analogRead() API is used directly.

const int micPin    = A1;      // Analog mic pin
const int SAMPLE_RATE = 16000; // Target sample rate (Hz)

void setup() {
  // The ADC is 12-bit by default on Portenta H7.
  // analogReadResolution(12) is optional (it's already the default).
}
```

> **Do NOT call** `analogReadResolution(10)` unless you intentionally want 10-bit range. If you do, update the ADC centre point from `2048` to `512` in the capture code.

---

## 3. Smoke Test — Verify Mic Wiring and ADC Response

### 3.1 Purpose

Confirm that the MAX4466 is correctly powered and wired, that the ADC reads a centred resting voltage, and that the amplitude increases detectably when sound is present.

### 3.2 Smoke Test Sketch

> **Source file:** [`src/analog_mic_test/analog_mic_test.ino`](../src/analog_mic_test/analog_mic_test.ino)

```cpp
// ================================================================
// Smoke Test: MAX4466 Analog Microphone
// Portenta H7 — Cortex-M7 core
//
// Wiring: MAX4466 OUT → A1, VCC → 3.3 V, GND → GND
// Pass criteria:
//   - Quiet:  ADC baseline ≈ 2048 ± 200, delta < 200
//   - Sound:  delta > 1000 when clapping within 30 cm
// ================================================================

const int microphonePin   = A1;
const int SOUND_THRESHOLD = 1000;  // Peak-to-peak delta for detection

void setup() {
  Serial.begin(115200);

  // Portenta H7 built-in RGB LEDs are ACTIVE LOW (LOW = ON)
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDR, HIGH);  // All LEDs off at start
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);

  Serial.println("=== MAX4466 Microphone Smoke Test ===");
  Serial.println("Quiet  → Blue LED ON");
  Serial.println("Sound  → Green LED ON");
  Serial.println("Headers: [Min]  [Max]  [Delta]  [State]");
}

void loop() {
  int mn = 4095, mx = 0;

  // Sample 10,000 ADC readings to capture peak-to-peak amplitude
  for (int i = 0; i < 10000; i++) {
    int v = analogRead(microphonePin);
    if (v < mn) mn = v;
    if (v > mx) mx = v;
  }

  int delta = mx - mn;  // Peak-to-peak amplitude (ADC counts)

  Serial.print("Min="); Serial.print(mn);
  Serial.print("  Max="); Serial.print(mx);
  Serial.print("  Delta="); Serial.print(delta);

  if (delta > SOUND_THRESHOLD) {
    digitalWrite(LEDG, LOW);   // GREEN ON — sound detected
    digitalWrite(LEDB, HIGH);
    digitalWrite(LEDR, HIGH);
    Serial.println("  [SOUND DETECTED]");
  } else {
    digitalWrite(LEDB, LOW);   // BLUE ON — idle/silent
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDR, HIGH);
    Serial.println("  [Idle]");
  }
}
```

### 3.3 How to Run

1. Open `src/analog_mic_test/analog_mic_test.ino` in Arduino IDE.
2. **Tools → Board → Arduino Portenta H7 (M7 core)**
3. **Tools → Port** → select your Portenta H7 COM port.
4. Click **Upload**.
5. Open **Tools → Serial Monitor** at **115200 baud**.

Optionally open **Tools → Serial Plotter** to visualise Min/Max/Delta as live graphs.

### 3.4 Expected Output

**Silent environment (no sound):**
```
Min=2041  Max=2056  Delta=15   [Idle]
Min=2039  Max=2058  Delta=19   [Idle]
Min=2044  Max=2051  Delta=7    [Idle]
```

**Clapping or speaking near the mic:**
```
Min=1124  Max=3208  Delta=2084  [SOUND DETECTED]
Min=1590  Max=3102  Delta=1512  [SOUND DETECTED]
```

### 3.5 Pass / Fail Criteria

| Check | Pass Condition |
|---|---|
| Resting ADC value | Min and Max both within 1800–2300 (centred near 2048) |
| Silent delta | delta < 200 in a quiet room |
| Sound response | delta > 1000 when clapping 30 cm from mic |
| LED behaviour | 🔵 Blue in silence, 🟢 Green when speaking |
| Stable readings | No constant `[SOUND DETECTED]` in a quiet room |

### 3.6 Adjusting Gain

| Symptom | Cause | Fix |
|---|---|---|
| Always `[SOUND DETECTED]` at rest | Gain too high | Rotate trimmer **counter-clockwise** |
| Never detects sound | Gain too low | Rotate trimmer **clockwise** |
| Min near 0 or Max near 4095 | ADC clipping | Reduce gain significantly |

---

## 4. Integration — Audio Capture in the Inference Pipeline

### 4.1 Role in the Full Pipeline

The microphone is the first stage in every inference sketch:

```
MAX4466 mic → analogRead(A0/A1) → audioBuffer[48000] → Mel Spectrogram → TFLite Inference
```

All production sketches (Drongonet, DrongoNet_Nano, DrongoNet_Edge, ARGUS) use the same `captureAudio()` function to fill a 48,000-sample `int16_t` buffer at exactly 16 kHz using a busy-wait timing loop.

### 4.2 The `captureAudio()` Function

> **Source reference:** [`src/DrongoNet/DrongoNet_Micro/Drongonet.ino`](../src/DrongoNet/DrongoNet_Micro/Drongonet.ino) — lines 535–547

```cpp
// ── Audio capture constants ───────────────────────────────────────────
const int MIC_PIN           = A1;     // Analog pin (A0 in ARGUS)
const int SAMPLE_RATE       = 16000;  // Hz — matches model training
const int AUDIO_DURATION_MS = 3000;   // 3-second capture window
const int BUFFER_SIZE       = (SAMPLE_RATE * AUDIO_DURATION_MS) / 1000; // 48 000

// Global audio buffer — 48 000 × 2 B = 96 KB in AXI SRAM BSS
static int16_t audioBuffer[BUFFER_SIZE];

// ── captureAudio() ────────────────────────────────────────────────────
// Acquires exactly BUFFER_SIZE samples at SAMPLE_RATE Hz using a
// busy-wait loop on micros(). Each sample is:
//   1. Read from the 12-bit ADC (0–4095)
//   2. Re-centred: (val - 2048) / 2048 → float in [-1.0, +1.0]
//   3. Scaled to int16 range: × 32767 → [-32767, +32767]
//
// Why busy-wait?
//   The Portenta H7 does not expose a hardware I²S or DMA-driven ADC
//   path in the standard Arduino API. Busy-waiting on micros() achieves
//   ±1 µs timing accuracy at 16 kHz (62.5 µs/sample), which is
//   sufficient for mel-spectrogram computation.
void captureAudio() {
  const unsigned long sampleInterval = 1000000UL / SAMPLE_RATE; // 62 µs
  unsigned long lastSample = micros();

  for (int i = 0; i < BUFFER_SIZE; i++) {
    // Busy-wait until exactly one sample interval has elapsed
    while (micros() - lastSample < sampleInterval) { /* spin */ }
    lastSample = micros();

    int val      = analogRead(MIC_PIN);
    // 12-bit ADC: midpoint = 2048, full-scale ±2048
    float sample = ((float)val - 2048.0f) / 2048.0f;  // → [-1.0, +1.0]
    audioBuffer[i] = (int16_t)(sample * 32767.0f);    // → int16 range
  }
}
```

### 4.3 Why 16 kHz?

16 kHz (Nyquist = 8 kHz) is the standard sampling rate for bird audio models because:

- Bird vocalisations span 0.5–8 kHz (most energy below 6 kHz)
- DRONGO dataset was recorded and trained at 16 kHz
- 16 kHz requires only 62.5 µs per sample — easily achieved by busy-wait
- 3 s × 16,000 = 48,000 samples = 93.75 KB (fits comfortably in AXI SRAM)

### 4.4 ADC Centring Explained

The Portenta H7 ADC outputs **unsigned** 12-bit values (0–4095) because the hardware measures voltage relative to GND. The MAX4466 output is an AC-coupled audio signal centred at VCC/2 ≈ 1.65 V, which maps to ADC midpoint ≈ 2048.

The conversion to signed `int16_t` is:

```
signed_sample = (adc_value - 2048) × 32767 / 2048
```

This maps:
- ADC = 0    → `int16` = −32767 (full negative)
- ADC = 2048 → `int16` = 0      (silence)
- ADC = 4095 → `int16` = +32767 (full positive)

### 4.5 RMS Energy Gate — Skipping Silent Windows

After capturing audio, all production sketches compute RMS energy before running mel / inference. Silent windows are skipped to save ~130 ms of compute and ~32 mJ of energy per cycle:

```cpp
// ── RMS Energy Gate ───────────────────────────────────────────────────
// Computes root-mean-square energy of the captured int16 buffer.
// If below RMS_THRESHOLD, the window is background noise / silence
// and inference is skipped entirely.
//
// NOTE: min-max normalisation would amplify even silence to [0,1]
// if not gated here — causing spurious inference results.

#define RMS_THRESHOLD 500.0f   // ADC int16 units; calibrate per environment

float rmsAccum = 0.0f;
for (int i = 0; i < BUFFER_SIZE; i++) {
  float s = (float)audioBuffer[i];
  rmsAccum += s * s;
}
float rms = sqrtf(rmsAccum / (float)BUFFER_SIZE);

if (rms < RMS_THRESHOLD) {
  Serial.print("[GATE] Silent — RMS="); Serial.print(rms, 1);
  Serial.println(" — inference skipped.");
  return;  // Skip all downstream processing
}
```

### 4.6 Pin Assignment Per Sketch

| Sketch | Mic Pin | Notes |
|---|---|---|
| `analog_mic_test.ino` | `A1` | Smoke test only |
| `DrongoNet_Nano.ino` | `A0` | Minimal inference test |
| `Drongonet.ino` | `A1` | Production BAD pipeline |
| `DrongoNet_Nano.ino` | `A1` | Nano profiling build |
| `DrongoNet_Edge.ino` | `A1` | 80-mel profiling build |
| `ARGUS.ino` | `A0` | Full cascade deployment |

---

## 5. Tuning and Calibration

### 5.1 RMS Threshold per Environment

Calibrate `RMS_THRESHOLD` by running **Drongonet.ino** or **DrongoNet_Nano.ino** and observing the `RMS=` value printed in silence:

| Environment | Typical silent RMS | Recommended threshold |
|---|---|---|
| Quiet indoor (anechoic) | 100–250 | 300 |
| Normal indoor room | 200–400 | 500 (default) |
| Outdoor (light wind) | 400–700 | 800 |
| Near fan / HVAC | 600–1000 | 1200 |
| Noisy outdoor (traffic) | 800–1500 | 2000 |

**Rule of thumb:** Set `RMS_THRESHOLD` ≈ 1.5 × observed silent RMS.

### 5.2 Gain Trimmer Adjustment

The MAX4466 breakout has a small blue potentiometer:
- **Clockwise** → more gain → larger ADC swing → lower threshold needed
- **Counter-clockwise** → less gain → smaller ADC swing → raise threshold

For a target ADC swing of 500–2000 counts during a bird call at 1 m distance, adjust gain until `Delta` (from the smoke test) falls in this range when a bird plays on a speaker at 50 cm.

---

## 6. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| ADC reads 0 always | VCC not connected or mic unpowered | Check MAX4466 VCC = 3.3 V |
| ADC reads 4095 always | Output saturated (gain too high or short) | Check OUT pin wiring; reduce gain |
| Baseline far from 2048 | Wrong pin or wrong analogRead pin number | Confirm `microphonePin = A1` matches wiring |
| No response to sound | Mic capsule not connected | Some breakouts require soldering the electret pad |
| `[GATE] Silent` always fires | RMS too high or threshold too low | Increase `RMS_THRESHOLD`; check for interference |
| `captureAudio()` returns garbage | Wrong ADC centring constant | Confirm Portenta ADC is 12-bit; use 2048 not 512 |

---

*Next →* **[Guide 02 — INA219 Current Sensor](Guide_02_INA219_Current_Sensor.md)**
