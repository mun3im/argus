# Guide 02 — INA219 Current & Power Sensor
## Arduino Portenta H7 Bird Recognition System

> **Component**: Adafruit INA219 High-Side I²C Current/Power Monitor  
> **Role in System**: Real-time energy profiling of each inference pipeline stage (mel computation, DrongoNet, MynaNet)  
> **Hardware**: Arduino Portenta H7 (STM32H747XI, Cortex-M7 @ 480 MHz)  

---

## Table of Contents

1. [Component Overview](#1-component-overview)
2. [Library Installation](#2-library-installation)
3. [Smoke Test — Verify I²C Communication and Sensor Readings](#3-smoke-test--verify-i2c-communication-and-sensor-readings)
4. [Integration — Per-Stage Energy Monitoring in the Inference Pipeline](#4-integration--per-stage-energy-monitoring-in-the-inference-pipeline)
5. [Interpreting Power Data](#5-interpreting-power-data)
6. [Troubleshooting](#6-troubleshooting)

---

## 1. Component Overview

### 1.1 What the INA219 Does

The INA219 is a high-side bidirectional DC current and power monitor that communicates over I²C. In this project it measures instantaneous power consumption of the Portenta H7 during each stage of the bird detection pipeline — audio capture, mel spectrogram computation, and TFLite Micro inference — enabling joule-level energy accounting.

| Parameter | Value |
|---|---|
| Interface | I²C (address 0x40 default) |
| Voltage measurement | Up to 26 V bus voltage |
| Current measurement | ±3.2 A (with 0.1 Ω shunt) |
| Resolution | 0.8 mA current, 4 mV voltage |
| Power measurement | Derived: `P = V × I` |
| Supply voltage | 3.0–5.5 V (use **3.3 V** with Portenta H7) |

### 1.2 Wiring

The INA219 is placed **in series** with the Portenta H7's power supply so that all current to the board flows through its 0.1 Ω shunt resistor.

| INA219 Pin | Portenta H7 Pin | Notes |
|---|---|---|
| VCC | 3.3 V | Sensor power |
| GND | GND | Common ground |
| SDA | GPIO 11 (SDA) | I²C data |
| SCL | GPIO 12 (SCL) | I²C clock |
| VIN+ | Power supply (+) | Current sense — positive rail in |
| VIN− | Portenta H7 VIN (+) | Current sense — positive rail out |

```
Power Supply (+) ── [INA219 VIN+] ── 0.1Ω shunt ── [INA219 VIN−] ── Portenta H7 VIN
Power Supply (−) ────────────────────────────────────────────────── Portenta H7 GND
```

> **Current polarity:** VIN+ must be connected to the **source** (power supply) and VIN− to the **load** (Portenta). Reversing them causes negative current readings.

---

## 2. Library Installation

### 2.1 Install via Arduino Library Manager

1. Open **Arduino IDE → Tools → Manage Libraries…**
2. Search: `Adafruit INA219`
3. Install: **Adafruit INA219** by Adafruit Industries

```
Library  : Adafruit INA219
Version  : ≥ 1.2.0
Author   : Adafruit Industries
Dependency: Adafruit BusIO (auto-installed)
```

When prompted, also install **Adafruit BusIO** — it is a required dependency for all Adafruit sensor libraries.

### 2.2 Required Headers

```cpp
#include <Adafruit_INA219.h>   // INA219 driver
#include <Wire.h>              // Arduino I²C (built-in)
```

Both headers are included in every production sketch in this project:
- [`src/DrongoNet/DrongoNet_Micro/Drongonet.ino`](../src/DrongoNet/DrongoNet_Micro/Drongonet.ino)
- [`src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino`](../src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino)
- [`src/DrongoNet/DrongoNet_Edge/DrongoNet_Edge.ino`](../src/DrongoNet/DrongoNet_Edge/DrongoNet_Edge.ino)
- [`src/ARGUS/ARGUS.ino`](../src/ARGUS/ARGUS.ino)

### 2.3 I²C Initialisation

Always call `Wire.begin()` before `ina219.begin()` in `setup()`. The INA219 is **optional** in all production sketches — if not found, a `[WARN]` message is printed and power data is skipped:

```cpp
Adafruit_INA219 ina219;
bool ina219_ok = false;

void setup() {
  Wire.begin();              // Initialise I²C bus first
  if (ina219.begin()) {
    ina219_ok = true;
    Serial.println("[OK]  INA219 initialised.");
  } else {
    Serial.println("[WARN] INA219 not found — power data unavailable.");
  }
}
```

---

## 3. Smoke Test — Verify I²C Communication and Sensor Readings

### 3.1 Purpose

Confirm that the INA219 is reachable on the I²C bus at address `0x40` and is returning plausible voltage and current readings for the Portenta H7's operating conditions.

### 3.2 Smoke Test Sketch

```cpp
// ================================================================
// Smoke Test: INA219 Current & Power Monitor
// Portenta H7 — Cortex-M7 core
//
// Wiring: VCC → 3.3 V, GND → GND, SDA → GPIO 11, SCL → GPIO 12
//         VIN+ → Power supply (+), VIN- → Portenta H7 VIN (+)
//
// Pass criteria:
//   - "[PASS] INA219 found" prints (sensor detected on I2C bus)
//   - Bus voltage: 3.1–3.5 V (3.3 V supply expected)
//   - Current: 50–350 mA (typical Portenta H7 range)
//   - No NaN or negative values
// ================================================================

#include <Adafruit_INA219.h>
#include <Wire.h>

Adafruit_INA219 ina219;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  delay(1000);

  Wire.begin();
  Serial.println("=== INA219 Current Sensor Smoke Test ===");

  if (!ina219.begin()) {
    Serial.println("[FAIL] INA219 not detected on I2C bus.");
    Serial.println("       Check: VCC=3.3V, SDA=GPIO11, SCL=GPIO12");
    Serial.println("       I2C address: 0x40 (default)");

    // Halt and blink red LED to signal failure
    pinMode(LEDR, OUTPUT);
    while (1) {
      digitalWrite(LEDR, LOW);  delay(200);
      digitalWrite(LEDR, HIGH); delay(200);
    }
  }

  Serial.println("[PASS] INA219 found at I2C address 0x40");
  Serial.println();
  Serial.println("  Bus Voltage | Shunt Voltage | Current | Power");
  Serial.println("  -----------+---------------+---------+-------");
}

void loop() {
  float busVoltage  = ina219.getBusVoltage_V();      // V
  float shuntVolt   = ina219.getShuntVoltage_mV();   // mV
  float current_mA  = ina219.getCurrent_mA();        // mA
  float power_mW    = ina219.getPower_mW();           // mW
  float loadVoltage = busVoltage + (shuntVolt / 1000.0f); // V

  Serial.print("  ");
  Serial.print(loadVoltage, 3); Serial.print(" V     | ");
  Serial.print(shuntVolt,   2); Serial.print(" mV          | ");
  Serial.print(current_mA,  2); Serial.print(" mA  | ");
  Serial.print(power_mW,    1); Serial.println(" mW");

  // Sanity checks
  if (current_mA < 0.0f) {
    Serial.println("  [WARN] Negative current — VIN+/VIN- may be swapped.");
  }
  if (busVoltage < 2.5f || busVoltage > 4.0f) {
    Serial.println("  [WARN] Bus voltage out of expected range (2.5–4.0 V).");
  }

  delay(500);
}
```

### 3.3 How to Run

1. Create a new Arduino sketch and paste the code above, **or** copy the INA219 section from an existing sketch.
2. **Tools → Board → Arduino Portenta H7 (M7 core)**
3. **Tools → Port** → select your Portenta H7 COM port.
4. Click **Upload**.
5. Open **Serial Monitor** at **115200 baud**.

### 3.4 Expected Output

```
=== INA219 Current Sensor Smoke Test ===
[PASS] INA219 found at I2C address 0x40

  Bus Voltage | Shunt Voltage | Current | Power
  -----------+---------------+---------+-------
  3.295 V     | 12.40 mV          | 124.00 mA  | 407.2 mW
  3.294 V     | 11.80 mV          | 118.00 mA  | 387.5 mW
  3.297 V     | 14.20 mV          | 142.00 mA  | 468.5 mW
```

### 3.5 Pass / Fail Criteria

| Check | Pass Condition |
|---|---|
| Sensor detected | `[PASS] INA219 found` printed on first line |
| No `[FAIL]` halt | Board does not halt in red-blink loop |
| Bus voltage | 3.0–3.6 V (≈ 3.3 V supply) |
| Current | 50–350 mA (Portenta idle to active) |
| Power | Positive, non-zero, finite |
| No `[WARN]` messages | Current positive, voltage in range |

---

## 4. Integration — Per-Stage Energy Monitoring in the Inference Pipeline

### 4.1 Energy Accounting Pattern

All production sketches use the same three-part energy accounting approach. The INA219 is read **after** each compute-heavy stage, combined with DWT cycle counts for microsecond-accurate time measurements:

```
Energy (µJ) = Power (mW) × Time (µs) / 1000
```

> **Source reference:** [`src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino`](../src/DrongoNet/DrongoNet_Nano/DrongoNet_Nano.ino) — power and energy section

### 4.2 Global State Variables

```cpp
#include <Adafruit_INA219.h>
#include <Wire.h>

Adafruit_INA219 ina219;
float  total_energy_mWh = 0.0f;   // Cumulative session energy
unsigned long lastPowerMs = 0;     // Timestamp of last INA219 read
bool   ina219_ok = false;          // True if sensor detected in setup()
```

### 4.3 Setup Initialisation

```cpp
void setup() {
  // ... (other setup code) ...

  Wire.begin();
  if (ina219.begin()) {
    ina219_ok   = true;
    lastPowerMs = millis();
    Serial.println("[OK]  INA219 initialised.");
  } else {
    Serial.println("[WARN] INA219 not found — power data unavailable.");
    // The sketch continues without power data — INA219 is non-blocking optional
  }
}
```

### 4.4 Per-Loop Power and Energy Read

```cpp
void loop() {
  // ... (audio capture, mel spectrogram, inference stages) ...
  //   melUs   = DWT-measured mel computation time in microseconds
  //   inferUs = DWT-measured inference time in microseconds

  // ── INA219 snapshot (after all compute stages) ────────────────────
  if (ina219_ok) {
    float busVoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float power_mW   = ina219.getPower_mW();

    // ── Per-stage energy from DWT cycle timing ─────────────────────
    // Energy (µJ) = Power (mW) × Duration (µs) / 1000
    float energy_mel_uJ   = power_mW * (melUs   / 1000.0f);
    float energy_infer_uJ = power_mW * (inferUs / 1000.0f);
    float energy_total_uJ = energy_mel_uJ + energy_infer_uJ;

    // ── Session energy accumulation ────────────────────────────────
    // Uses wall-clock millis() for total session energy (coarser).
    unsigned long now = millis();
    float dt_hours    = (now - lastPowerMs) / 3600000.0f;
    lastPowerMs       = now;
    if (power_mW > 0.0f)
      total_energy_mWh += power_mW * dt_hours;

    // ── Print power report ─────────────────────────────────────────
    Serial.println(F("\n─────────── Power & Energy ──────────────"));
    Serial.print(F("  Bus Voltage   : ")); Serial.print(busVoltage, 3);
    Serial.println(F(" V"));
    Serial.print(F("  Current       : ")); Serial.print(current_mA, 2);
    Serial.println(F(" mA"));
    Serial.print(F("  Power         : ")); Serial.print(power_mW, 1);
    Serial.println(F(" mW"));
    Serial.print(F("  Energy (mel)  : ")); Serial.print(energy_mel_uJ, 1);
    Serial.println(F(" µJ"));
    Serial.print(F("  Energy (infer): ")); Serial.print(energy_infer_uJ, 1);
    Serial.println(F(" µJ"));
    Serial.print(F("  Energy (total): ")); Serial.print(energy_total_uJ, 1);
    Serial.println(F(" µJ  (mel + infer stages only)"));
    Serial.print(F("  Session Total : ")); Serial.print(total_energy_mWh, 7);
    Serial.println(F(" mWh"));
    Serial.println(F("─────────────────────────────────────────"));
  }
}
```

### 4.5 DWT Cycle Counter — Prerequisite for Accurate Energy

The energy calculation depends on `melUs` and `inferUs` obtained from the DWT (Data Watchpoint and Trace) cycle counter. Enable and use it in `setup()` and `loop()`:

```cpp
// ── DWT Cycle Counter macros ──────────────────────────────────────────
// At 480 MHz: 1 cycle = 2.083 ns → DWT_US(cycles) = cycles / 480.0
#define DWT_ENABLE() do { \
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; \
  DWT->CYCCNT = 0; \
  DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk; \
} while(0)
#define DWT_RESET()  do { DWT->CYCCNT = 0; } while(0)
#define DWT_CYCLES() (DWT->CYCCNT)
#define DWT_US(cyc)  ((float)(cyc) / 480.0f)

// In setup():
DWT_ENABLE();

// In loop() — wrap around each timed block:
DWT_RESET();
computeMelSpectrogram(audioBuffer, BUFFER_SIZE, melFeatures);
uint32_t melCycles = DWT_CYCLES();
float melUs = DWT_US(melCycles);

DWT_RESET();
interpreter->Invoke();
uint32_t inferCycles = DWT_CYCLES();
float inferUs = DWT_US(inferCycles);
```

### 4.6 Typical Power Profile

| Pipeline Stage | Duration | Typical Power | Energy |
|---|---|---|---|
| Audio capture (idle CPU, ADC polling) | 3000 ms | ~150 mW | ~450 mJ |
| DrongoNet mel (16-bin, N=1024 FFT) | ~12 ms | ~800 mW | ~9.6 mJ |
| DrongoNet inference (919 params) | ~40 ms | ~800 mW | ~32 mJ |
| MynaNet mel (64-bin, N=512 FFT) | ~25 ms | ~800 mW | ~20 mJ |
| MynaNet inference (DS-CNN, 267 KB) | ~100 ms | ~800 mW | ~80 mJ |
| **Per cycle — BAD gate only (no bird)** | ~3052 ms | — | ~492 mJ |
| **Per cycle — full cascade (bird detected)** | ~3177 ms | — | ~592 mJ |

> Values are approximate. Use your own INA219 readings for device-specific calibration.

---

## 5. Interpreting Power Data

### 5.1 What the Numbers Mean

```
Energy (µJ) = Power (mW) × Time (µs) / 1000
```

**Example from DrongoNet_Nano.ino Serial output:**
```
─────────── Power & Energy ──────────────
  Bus Voltage   : 3.285 V
  Current       : 245.30 mA
  Power         : 805.70 mW
  Energy (mel)  : 10030.7 µJ   ← 805.7 mW × 12,450 µs / 1000
  Energy (infer): 33210.1 µJ   ← 805.7 mW × 41,234 µs / 1000
  Energy (total): 43240.8 µJ
  Session Total : 0.0003421 mWh
─────────────────────────────────────────
```

### 5.2 Energy Efficiency Comparison

The cascade architecture saves energy by skipping MynaNet when no bird is detected:

| Scenario | DrongoNet only | + MynaNet activated |
|---|---|---|
| Compute energy per cycle | ~41.6 mJ | ~141.6 mJ |
| Savings from gating | — | ~100 mJ saved per silent cycle |
| Savings at 10% bird activity | — | ~90 mJ per cycle average |

### 5.3 Battery Life Estimation

For a 3.7 V, 2000 mAh Li-Po battery (7400 mJ):

```
Cycles per charge ≈ 7,400,000 mJ / 492 mJ ≈ 15,040 cycles
Duration          ≈ 15,040 × 3 s ≈ 45,120 s ≈ 12.5 hours
```

---

## 6. Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| `INA219 not found` | Wrong I²C address or wiring | Confirm SDA=GPIO11, SCL=GPIO12, address=0x40 |
| Negative current readings | VIN+/VIN− reversed | Swap the power supply leads at the INA219 |
| Current reads 0 mA | No load or shunt bypassed | Ensure INA219 is in series with Portenta power rail |
| Bus voltage reads NaN | Library version issue | Re-install Adafruit INA219 library |
| Readings unstable / jittery | Long I²C wires or no pull-ups | Shorten wires; add 4.7 kΩ pull-ups on SDA/SCL to 3.3 V |
| Power reads 0 but current ≠ 0 | ina219.getPower_mW() needs calibration | Call `ina219.setCalibration_32V_2A()` in setup() |
| Energy values too large | INA219 not read on every cycle | Ensure `lastPowerMs` is updated every loop |

### 6.1 Force Calibration

If power readings are zero but current is non-zero:

```cpp
void setup() {
  Wire.begin();
  ina219.begin();
  ina219.setCalibration_32V_2A();  // Force full-range calibration
}
```

---

*Previous →* **[Guide 01 — Microphone](Guide_01_Microphone.md)**  
*Next →* **[Guide 03 — DrongoNet BAD CNN Deployment](Guide_03_DrongoNet_BAD_CNN.md)**
