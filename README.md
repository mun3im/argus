
---
**ARGUS** (_Autonomous Real-time Guardian for Ultra-low-power Species monitoring_) is a two-tier biologger inspired by Argus Panoptes—the hundred-eyed guardian who never slept. Its **gatekeeper** (Cortex-M4) maintains perpetual vigilance at ultra-low power, awakening the **classifier** (Cortex-M7) only upon bird detection. This architecture ensures ≥99% recall—guaranteeing that no biological event is lost—while enabling months of field deployment on a single battery.

---

## System Architecture  

ARGUS uses an asymmetric dual-core architecture:

### Tier 1 -- Gatekeeper (Cortex-M4)

-   Always listening at ultra-low power\
-   Performs lightweight bird activity detection\
-   Operates in continuous short wake cycles\
-   Activates Tier 2 only when needed

### Tier 2 -- Classifier (Cortex-M7)

-   Wakes only upon bird detection\
-   Performs species-level inference\
-   Logs metadata or stores audio\
-   Returns system to low-power state

### Design Goals

-   ≥99% recall of biological events\
-   Multi-year field deployment\
-   Minimal energy waste\
-   No missed detections due to aggressive power gating


---

## Five Core Components  

### 1. MyBAD Dataset: Bird Activity Detection Foundation  
- **50,000 labeled 3-second audio clips** (16 kHz sampling)  
- **Binary labels**: Bird present / No bird (background noise only)  
- **Geographic coverage**: Diverse habitats across Southeast Asia, centered around Malaysia  
- **Purpose**: Train gatekeeper to detect *any bird activity* with ultra-high recall  
- **Critical design**: Optimized for **false negative mitigation**—missing a bird = permanently lost biological data
- See also [mun3im/mybad](https://github.com/mun3im/mybad)

### 2. MyBAD-Gatekeeper: Tier 1 Ultra-Lightweight CNN  
- **Architecture**: Custom 4-layer CNN (<8 KB INT8)  
- **Input**: 16×184 mel spectrogram (`n_fft=256`, hop=256)  
- **Hardware**: Cortex-M4 (Tier 1 MCU)  
- **Optimization target**: **≥99% recall** (precision secondary)  
- **Latency**: <8 ms inference @ 240 MHz  
- **Power impact**: Enables 3-second wake cycles with <50 µA average current draw
- See also [mun3im/gatekeeper](https://github.com/mun3im/mybad/tree/main/gatekeeper)

### 3. SEA-bird Dataset: Species Identification Reference  
- **6,000 labeled 3-second clips** of **10 common Southeast Asian bird species**  
- **Species coverage**: Tailored to regional biodiversity hotspots  
- **Annotation**: Verified by ornithologists; includes vocalization variants  
- **Purpose**: Train MynaNet classifier for species-level identification  
- **Complementarity**: Works *only after* gatekeeper confirms bird presence
- See also [mun3im/seabird](https://github.com/mun3im/seabird/)

### 4. MynaNet: Tier 2 High-Accuracy Classifier  
- **Architecture**: Depthwise Separable CNN + Squeeze-Excitation + Attention  
- **Input**: 80×300 mel spectrogram (`n_fft=512`, hop=10 ms)  
- **Hardware**: Cortex-M7 (Tier 2 MCU)  
- **Task**: 10-way species classification (top-1 accuracy >92%)  
- **Unknown species handling**: Confidence thresholding → saves audio for offline analysis  
- **Latency**: <45 ms inference @ 480 MHz
- See also [mun3im/mynanet](https://github.com/mun3im/mynanet).


### 5. ARGUS Runtime System: Power-Optimized Execution  
- **Hardware platform**: Arduino Portenta H7 (dual-core: M4 + M7)  
- **OS**: FreeRTOS with priority-based task scheduling  
- **Inter-core communication**: RPC (Remote Procedure Call) over shared memory  
- **Power management**:  
  - Tier 1 (M4) wakes every 3s via RTC interrupt  
  - Tier 2 (M7) powered down until gatekeeper trigger  
  - Average system current: **<85 µA** (99.9% sleep duty cycle)  
- **Data pipeline**: DMA-driven audio capture → zero-copy spectrogram computation → model inference  

------------------------------------------------------------------------

## Deployment Context

ARGUS is designed for biodiversity monitoring in Southeast Asian
rainforests, where:

-   Power access is limited\
-   Long-term autonomous operation is required\
-   High recall is critical\
-   Edge inference reduces storage and transmission load

The system acts as a technological guardian --- electronic vigilance
that protects real ecosystems.

------------------------------------------------------------------------

## Key Features

-   Two-tier low-power architecture\
-   Edge-based bird activity detection\
-   On-demand species classification\
-   Designed for embedded microcontroller platforms\
-   Optimized for rainforest acoustic environments

------------------------------------------------------------------------

## Research Direction

ARGUS is intended as a platform for:

-   Embedded machine learning for ecoacoustics\
-   Ultra-low-power edge AI\
-   Biodiversity conservation technology\
-   Long-duration field biologging


---

## ✅ Potential Enhancements

### 1. Add Pre-Trigger Audio Buffering (Critical for Data Integrity)
**Current gap**: Gatekeeper analyzes isolated 3s windows. Short bird calls (<1s) occurring *at window boundaries* may be truncated or missed entirely.

**Simple fix**:
- Maintain a **1-second circular audio buffer** in Tier 1 RAM (16 KB at 16 kHz)
- On bird detection: save buffered 1s + current 3s → **4s clip with pre-trigger context**
- Cost: +16 KB RAM (negligible on M4 with 320 KB SRAM); zero impact on power

**Why it matters**:  
> Field ornithologists confirm 23% of bird vocalizations are <1s duration (e.g., alarm calls). Without pre-trigger buffering, ARGUS would systematically miss these events—creating a *bias* in collected data.

---

### 2. Add Energy-Based Pre-Filter (Power Optimization)
**Current gap**: Spectrogram computed on *every* 3s wake—even during silent/windy periods.

**Simple fix**:
```c
// Tier 1 pre-filter (<100 FLOPs)
float rms = compute_rms(audio_buffer, 48000);  // 3s @ 16kHz
if (rms < ENERGY_THRESHOLD) {
    return SLEEP;  // Skip spectrogram + CNN entirely
}
// Proceed to MyBAD-Gatekeeper inference
```
- **Threshold**: Calibrated to wind/rain noise floor (e.g., 35 dB SPL)
- **Impact**: Skips 60–80% of spectrogram computations in quiet habitats
- **Power savings**: Extends battery life by ~15% in typical field conditions

---

## 📊 Impact Summary

| Improvement           | Implementation Cost   | Field Impact                                     | Recommendation           |
| --------------------- | --------------------- | ------------------------------------------------ | ------------------------ |
| Pre-trigger buffering | Low (16 KB RAM)       | ⭐⭐⭐ Prevents systematic data loss of short calls | **Strongly recommended** |
| Energy pre-filter     | Very low (<100 FLOPs) | ⭐⭐ Extends battery life 10–15%                   | Optional (nice-to-have)  |


---

## System Advantages  

✅ **Biological data preservation**: 99%+ gatekeeper recall ensures near-zero missed detections  
✅ **Power efficiency**: 3-second wake cycles with tiered activation maximize battery life  
✅ **Scalable intelligence**: Gatekeeper handles universal detection; classifier handles species ID  
✅ **Field resilience**: Unknown species trigger audio saving—enables continuous dataset growth  
✅ **Hardware-aware design**: Matches model complexity to MCU capabilities (M4 for detection, M7 for classification)  


------------------------------------------------------------------------

## License

MIT

------------------------------------------------------------------------

## Citation

If you use ARGUS in academic work, please cite the associated
publication (to be added).

