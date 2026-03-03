# ARGUS Repository Sitemap

**Last Updated**: 2026-02-28
**Purpose**: Documentation of ARGUS multi-repository structure and relationships

---

## Repository Architecture Overview

ARGUS is organized as **4 independent repositories** with clear separation of concerns:

```
github.com/mun3im/
├── mybad/              # Dataset + Reference Model (Activity Detection)
├── mygardenbird/       # Dataset (Species Identification)
├── mynanet/            # Model (Species Classifier)
└── argus/              # System Integration (Dual-Core Deployment)
```

### Design Principles

1. **Modularity**: Each repository is independently useful
2. **Reusability**: Components can be used without the full system
3. **Standard Pattern**: Datasets include reference models (like ImageNet + ResNet)
4. **Clear Ownership**: Each repo has a single, well-defined purpose

---

## Repository 1: `mybad`

**GitHub**: `github.com/mun3im/mybad`
**Purpose**: Bird Activity Detection Dataset + Reference Model
**Paper**: MyBAD Dataset Paper

### What Lives Here

```
mybad/
├── dataset/
│   ├── curation/
│   │   ├── positive-label-curation/      # Scripts for curating positive samples
│   │   ├── negative-sample-curation/     # Scripts for curating negative samples
│   │   └── dataset_final_checks/         # Quality control and validation
│   ├── splits/
│   │   ├── train.csv                     # 40,000 samples (80%)
│   │   ├── val.csv                       # 5,000 samples (10%)
│   │   └── test.csv                      # 5,000 samples (10%)
│   ├── statistics/
│   │   ├── class_distribution.json
│   │   └── source_analysis.json
│   └── download_scripts/
│       ├── download_from_zenodo.py
│       └── verify_checksums.py
├── mybadnet/                              # REFERENCE MODEL (KEEP HERE)
│   ├── architecture.py                    # 4-layer CNN implementation
│   ├── train.py                           # Training script
│   ├── quantize.py                        # INT8 quantization pipeline
│   ├── evaluate.py                        # Evaluation on test set
│   ├── pretrained/
│   │   ├── mybadnet_fp32.keras
│   │   ├── mybadnet_int8.tflite
│   │   └── metrics.json
│   └── README.md
├── baselines/                             # Benchmark comparisons
│   ├── resnet50/
│   │   ├── train_resnet50.py
│   │   └── results.txt
│   ├── vgg16/
│   ├── mobilenetv3/
│   └── efficientnetb0/
├── validation/
│   ├── mybad_validation/                  # Cross-validation experiments
│   ├── analysis_results.txt
│   └── confusion_matrices/
└── README.md
```

### README Template

```markdown
# MyBAD: Malaysian Bird Activity Detection Dataset

50,000 labeled 3-second audio clips for binary bird activity detection.

## Dataset Details
- **Total Samples**: 50,000 (25,000 bird, 25,000 no-bird)
- **Format**: 16 kHz, mono, 3-second WAV
- **Split**: 80/10/10 (train/val/test, stratified)
- **Sources**: Xeno-canto (positive), BirdCLEF, ESC-50, FSC22, DCASE2018 (negative)

## MyBADNet: Reference Implementation

This repository includes **MyBADNet**, a 4-layer CNN optimized for edge deployment:
- Model size: <8 KB (INT8)
- Target recall: ≥99%
- Designed for ARM Cortex-M4 @ 240 MHz

### Quick Start
```bash
# Download dataset
python dataset/download_scripts/download_from_zenodo.py

# Train MyBADNet
python mybadnet/train.py --config configs/default.yaml

# Evaluate
python mybadnet/evaluate.py --model pretrained/mybadnet_int8.tflite
```

## Baseline Benchmarks
- ResNet50: 99.68% accuracy
- VGG16: 99.67% accuracy
- MobileNetV3-Small: 98.81% accuracy
- MyBADNet: See `mybadnet/README.md`

## Related Repositories
- **MynaNet** (species classifier): [github.com/mun3im/mynanet](https://github.com/mun3im/mynanet)
- **MyGardenBird** (species dataset): [github.com/mun3im/mygardenbird](https://github.com/mun3im/mygardenbird)
- **ARGUS** (dual-core system): [github.com/mun3im/argus](https://github.com/mun3im/argus)

## Citation
```bibtex
@article{zabidi2026mybad,
  title={MyBAD: Malaysian Bird Activity Detection Dataset},
  author={Zabidi, Mun3im},
  journal={...},
  year={2026}
}
```
```

### Key Points
- ✅ MyBADNet **stays in this repo** (reference implementation)
- ✅ Includes baseline benchmarks (ResNet50, VGG16, etc.)
- ✅ Complete dataset curation pipeline
- ✅ Standalone usage: "I want bird activity detection" → clone this only

---

## Repository 2: `mygardenbird`

**GitHub**: `github.com/mun3im/mygardenbird`
**Purpose**: Bird Species Dataset (10 Southeast Asian Species)
**Paper**: MyGardenBird Dataset Paper (or part of ARGUS paper)

### What Lives Here

```
mygardenbird/
├── dataset/
│   ├── species/
│   │   ├── asian_koel/              # 600 clips
│   │   ├── spotted_dove/
│   │   ├── collared_kingfisher/
│   │   ├── common_tailorbird/
│   │   ├── coppersmith_barbet/
│   │   ├── large_tailed_nightjar/
│   │   ├── white_breasted_waterhen/
│   │   ├── white_throated_kingfisher/
│   │   ├── common_iora/
│   │   └── olive_backed_sunbird/
│   ├── splits/
│   │   ├── train.csv                # 4,800 samples (80%)
│   │   ├── val.csv                  # 600 samples (10%)
│   │   └── test.csv                 # 600 samples (10%)
│   │   └── mip_optimization/        # Source-based split via MIP
│   ├── metadata/
│   │   ├── species_info.json        # Scientific names, descriptions
│   │   ├── recording_sources.csv    # 1,124 Xeno-canto recordings
│   │   └── annotation_quality.json
│   └── download_scripts/
├── analysis/
│   ├── species_distribution/
│   ├── acoustic_features/
│   │   ├── frequency_analysis.py
│   │   └── temporal_patterns.py
│   └── visualization/
└── README.md
```

### README Template

```markdown
# MyGardenBird: Southeast Asian Garden Bird Dataset

6,000 annotated 3-second audio clips representing 10 Malaysian garden bird species.

## Dataset Details
- **Total Samples**: 6,000 (600 per species)
- **Species**: 10 (see list below)
- **Format**: 16 kHz, mono, 3-second WAV
- **Split**: 80/10/10 via MIP (prevents data leakage)
- **Sources**: 1,124 unique FLAC recordings from Xeno-canto

## Species List
1. Asian Koel (*Eudynamys scolopaceus*)
2. Spotted Dove (*Spilopelia chinensis*)
3. Collared Kingfisher (*Todiramphus chloris*)
4. Common Tailorbird (*Orthotomus sutorius*)
5. Coppersmith Barbet (*Psilopogon haemacephalus*)
6. Large-tailed Nightjar (*Caprimulgus macrurus*)
7. White-breasted Waterhen (*Amaurornis phoenicurus*)
8. White-throated Kingfisher (*Halcyon smyrnensis*)
9. Common Iora (*Aegithina tiphia*)
10. Olive-backed Sunbird (*Cinnyris jugularis*)

## Usage with MynaNet
This dataset is designed for use with **MynaNet**, a hybrid CNN species classifier:
```bash
git clone https://github.com/mun3im/mynanet.git
python mynanet/train.py --dataset /path/to/mygardenbird
```

## Related Repositories
- **MynaNet** (classification model): [github.com/mun3im/mynanet](https://github.com/mun3im/mynanet)
- **MyBAD** (activity detection): [github.com/mun3im/mybad](https://github.com/mun3im/mybad)
- **ARGUS** (dual-core system): [github.com/mun3im/argus](https://github.com/mun3im/argus)
```

### Key Points
- ✅ Dataset only (no model implementation)
- ✅ MIP-based splits prevent data leakage
- ✅ Points to `mynanet` repo for model

---

## Repository 3: `mynanet`

**GitHub**: `github.com/mun3im/mynanet`
**Purpose**: Species Classification Model
**Paper**: MynaNet Model Paper (or part of ARGUS paper)

### What Lives Here

```
mynanet/
├── model/
│   ├── architecture.py              # DS-CNN + SE + Residual + Attention
│   ├── layers/
│   │   ├── depthwise_separable.py
│   │   ├── squeeze_excitation.py
│   │   └── multi_head_attention.py
│   ├── train.py
│   ├── quantize.py
│   ├── evaluate.py
│   └── configs/
│       ├── default.yaml
│       └── hyperparameters.yaml
├── pretrained/
│   ├── mynanet_fp32.keras
│   ├── mynanet_int8.tflite          # 433.9 KB
│   ├── metrics.json                 # 95.83% accuracy
│   └── training_history.json
├── experiments/
│   ├── hyperparameter_tuning/
│   │   ├── dropout_mixup_ablation/
│   │   └── learning_rate_search/
│   ├── ablation_studies/
│   │   ├── without_attention/
│   │   ├── without_se_blocks/
│   │   └── without_residuals/
│   └── results_summary.md
├── deployment/
│   ├── tflite/
│   │   ├── convert_to_tflite.py
│   │   └── benchmark_tflite.py
│   └── benchmarks/
│       ├── latency_analysis.py
│       └── quantization_analysis.py
└── README.md
```

### README Template

```markdown
# MynaNet: Hybrid CNN for Bird Species Classification

A lightweight deep learning model for classifying 10 Southeast Asian bird species.

## Architecture
- **Base**: Depthwise Separable Convolutions
- **Enhancements**:
  - Squeeze-Excitation blocks (1:16 reduction)
  - Residual connections (all 4 blocks)
  - Multi-head self-attention (2 heads, 88-dim)
- **Parameters**: 327,908 (323,200 trainable)
- **Model Size**: 433.9 KB (INT8)

## Performance
- **Top-1 Accuracy**: 95.83% (seed 42, dropout 0.05, mixup 0.25)
- **Quantization Drop**: 0.00% (FP32 → INT8)
- **Estimated Latency**: <45 ms on Cortex-M7 @ 480 MHz
- **Training**: 140 epochs (100 warmup @ LR=0.001, 40 fine-tune @ LR=1e-5)

## Quick Start

### Training
```bash
# Clone MyGardenBird dataset
git clone https://github.com/mun3im/mygardenbird.git

# Train MynaNet
python model/train.py \
  --dataset ../mygardenbird/dataset \
  --config configs/default.yaml \
  --seed 42
```

### Inference
```python
from model.architecture import MynaNet
import tensorflow as tf

model = tf.keras.models.load_model('pretrained/mynanet_fp32.keras')
prediction = model.predict(audio_spectrogram)
```

### Quantization
```bash
python model/quantize.py \
  --input pretrained/mynanet_fp32.keras \
  --output pretrained/mynanet_int8.tflite
```

## Related Repositories
- **MyGardenBird** (training dataset): [github.com/mun3im/mygardenbird](https://github.com/mun3im/mygardenbird)
- **ARGUS** (edge deployment): [github.com/mun3im/argus](https://github.com/mun3im/argus)
```

### Key Points
- ✅ Model implementation only
- ✅ Trained on MyGardenBird dataset
- ✅ Pretrained weights included
- ✅ Standalone usage: "I want species classification" → clone this + mygardenbird

---

## Repository 4: `argus`

**GitHub**: `github.com/mun3im/argus`
**Purpose**: Dual-Core System Integration (Complete ARGUS System)
**Paper**: ARGUS Paper (IEEE Conference)

### What Lives Here

```
argus/
├── models/                          # Git submodules (NOT implementations)
│   ├── mybadnet/                    → submodule: github.com/mun3im/mybad
│   └── mynanet/                     → submodule: github.com/mun3im/mynanet
├── datasets/                        # Git submodules or links
│   ├── mybad/                       → submodule: github.com/mun3im/mybad
│   └── mygardenbird/                → submodule: github.com/mun3im/mygardenbird
├── deployment/
│   ├── portenta_h7/
│   │   ├── m4_gatekeeper/           # MyBADNet on Cortex-M4 @ 240 MHz
│   │   │   ├── firmware/
│   │   │   ├── mybadnet_deployment.ino
│   │   │   └── audio_capture.c
│   │   ├── m7_classifier/           # MynaNet on Cortex-M7 @ 480 MHz
│   │   │   ├── firmware/
│   │   │   ├── mynanet_deployment.ino
│   │   │   └── species_inference.c
│   │   ├── shared_memory/           # RPC communication
│   │   │   ├── rpc_protocol.h
│   │   │   ├── audio_buffer.c
│   │   │   └── dma_config.c
│   │   ├── integration/
│   │   │   ├── dual_core_main.ino
│   │   │   └── freertos_tasks.c
│   │   └── README.md
│   └── alternative_platforms/       # Optional: ESP32-S3, STM32H7
├── benchmarks/
│   ├── latency/
│   │   ├── m4_mybadnet_latency.py
│   │   ├── m7_mynanet_latency.py
│   │   └── end_to_end_latency.py
│   ├── power/
│   │   ├── power_profiling_ppk2.py
│   │   ├── dual_core_power.csv
│   │   └── battery_life_calculator.py
│   ├── accuracy/
│   │   ├── on_device_validation.py
│   │   └── field_testing_results.csv
│   └── results/
│       └── benchmark_summary.md
├── docs/
│   ├── paper/
│   │   ├── zabidi2026argus.tex
│   │   ├── figures/
│   │   └── references.bib
│   ├── hardware_setup/
│   │   ├── bill_of_materials.md
│   │   ├── wiring_diagram.pdf
│   │   └── enclosure_design.stl
│   └── deployment_guide.md
├── tools/
│   ├── audio_capture/
│   ├── data_logging/
│   └── visualization/
├── .gitmodules                      # Submodule configuration
└── README.md
```

### Git Submodules Setup

```bash
# In argus/.gitmodules
[submodule "models/mybadnet"]
    path = models/mybadnet
    url = https://github.com/mun3im/mybad.git
    branch = main

[submodule "models/mynanet"]
    path = models/mynanet
    url = https://github.com/mun3im/mynanet.git
    branch = main

[submodule "datasets/mybad"]
    path = datasets/mybad
    url = https://github.com/mun3im/mybad.git
    branch = main

[submodule "datasets/mygardenbird"]
    path = datasets/mygardenbird
    url = https://github.com/mun3im/mygardenbird.git
    branch = main
```

### README Template

```markdown
# ARGUS: Dual-Core Edge Audio Recognition System

A heterogeneous dual-core deployment strategy for bird audio recognition on Arduino Portenta H7, combining activity detection (MyBADNet on M4) and species classification (MynaNet on M7).

## System Architecture

```
┌─────────────────────────────────────┐
│    Arduino Portenta H7             │
│                                     │
│  ┌──────────────┐  ┌─────────────┐ │
│  │ Cortex-M4    │  │ Cortex-M7   │ │
│  │ @ 240 MHz    │  │ @ 480 MHz   │ │
│  │              │  │             │ │
│  │  MyBADNet    │◄─┤  MynaNet    │ │
│  │  (<8 KB)     │  │  (434 KB)   │ │
│  │  <8 ms       │  │  <45 ms     │ │
│  │  ≥99% recall │  │  95.83% acc │ │
│  │              │  │             │ │
│  │ Gatekeeper   │  │ Classifier  │ │
│  └──────┬───────┘  └──────▲──────┘ │
│         │                 │         │
│         └─────RPC/DMA─────┘         │
└─────────────────────────────────────┘
```

## Key Features
- **Ultra-low power**: <85 µA average (99.9% sleep duty cycle)
- **Two-stage pipeline**: Activity detection gates species classification
- **Heterogeneous deployment**: M4 for continuous monitoring, M7 for classification
- **Multi-month battery life**: Estimated 3-6 months on 2000 mAh battery

## Performance Metrics
| Module | Core | Latency | Model Size | Accuracy |
|--------|------|---------|------------|----------|
| MyBADNet | M4 @ 240 MHz | <8 ms | <8 KB | ≥99% recall |
| MynaNet | M7 @ 480 MHz | <45 ms | 433.9 KB | 95.83% top-1 |

## Quick Start

### 1. Clone with Submodules
```bash
git clone --recursive https://github.com/mun3im/argus.git
cd argus

# If already cloned without --recursive:
git submodule update --init --recursive
```

### 2. Train Models (Optional - pretrained included)
```bash
# Train MyBADNet on MyBAD dataset
cd models/mybadnet
python mybadnet/train.py

# Train MynaNet on MyGardenBird dataset
cd ../mynanet
python model/train.py --dataset ../../datasets/mygardenbird
```

### 3. Deploy to Portenta H7
```bash
cd deployment/portenta_h7

# Option A: Arduino IDE
# Open dual_core_main.ino and upload to both cores

# Option B: PlatformIO
pio run --target upload
```

### 4. Benchmark System
```bash
cd benchmarks

# Measure latency
python latency/end_to_end_latency.py

# Measure power consumption (requires Nordic PPK2)
python power/power_profiling_ppk2.py
```

## Hardware Requirements
- Arduino Portenta H7 (~$100)
- MEMS Microphone (Vesper VM1010 or Knowles SPH0645, ~$7-25)
- MicroSD card for logging
- Li-Ion battery (2000-5000 mAh, 3.7V)
- Optional: Nordic PPK2 for power profiling (~$90)

## Repository Structure
- `models/`: Git submodules to MyBADNet and MynaNet implementations
- `datasets/`: Git submodules to MyBAD and MyGardenBird datasets
- `deployment/`: Dual-core firmware and hardware integration
- `benchmarks/`: Latency, power, and accuracy measurements
- `docs/`: Paper, hardware setup, deployment guides

## Related Repositories
This is a multi-repository project:
- **MyBAD** (dataset + model): [github.com/mun3im/mybad](https://github.com/mun3im/mybad)
- **MyGardenBird** (dataset): [github.com/mun3im/mygardenbird](https://github.com/mun3im/mygardenbird)
- **MynaNet** (model): [github.com/mun3im/mynanet](https://github.com/mun3im/mynanet)
- **ARGUS** (this repo): System integration

## Citation
```bibtex
@inproceedings{zabidi2026argus,
  title={ARGUS: A Dual-Core Edge Audio Recognition System for Bird Activity Detection and Species Classification},
  author={Zabidi, Mun3im},
  booktitle={IEEE Conference},
  year={2026}
}
```

## License
[Specify license]
```

### Key Points
- ✅ **Integration only** (firmware, deployment, benchmarks)
- ✅ Uses git submodules for models and datasets
- ✅ ARGUS paper lives here
- ✅ Hardware deployment guides
- ❌ Does NOT reimplement models (links to them)

---

## Cross-Repository Relationships

### Dependency Graph

```
┌─────────────┐
│   mybad     │ ← MyBAD Dataset Paper
│  + mybadnet │ ← Reference model included
└──────┬──────┘
       │
       ├─────────────┐
       │             │
┌──────▼──────┐  ┌──▼──────────┐
│mygardenbird │  │   mynanet   │ ← MynaNet Model Paper
│  dataset    │  │   model     │
└──────┬──────┘  └──┬──────────┘
       │            │
       └────┬───────┘
            │
       ┌────▼─────┐
       │  argus   │ ← ARGUS System Paper (IEEE)
       │integration│ ← Uses all above via submodules
       └──────────┘
```

### Citation Relationships

- **ARGUS paper** cites:
  - MyBAD dataset (`mybad` repo)
  - MyGardenBird dataset (`mygardenbird` repo)
  - MyBADNet model (part of `mybad` repo)
  - MynaNet model (`mynanet` repo)

- **MyBAD dataset paper** references:
  - ARGUS system for deployment (`argus` repo)
  - MynaNet for comparison (`mynanet` repo)

- **MynaNet paper** references:
  - MyGardenBird dataset (`mygardenbird` repo)
  - ARGUS system for deployment (`argus` repo)

---

## Use Cases & User Journeys

### Use Case 1: "I want bird activity detection only"

```bash
# Clone mybad repository
git clone https://github.com/mun3im/mybad.git
cd mybad

# Download dataset
python dataset/download_scripts/download_from_zenodo.py

# Train MyBADNet (or use pretrained)
python mybadnet/train.py

# Deploy to single microcontroller (M4 standalone)
python mybadnet/deploy_tflite.py --target stm32
```

**Result**: Standalone activity detection system, no species classification needed.

---

### Use Case 2: "I want species classification only"

```bash
# Clone both repositories
git clone https://github.com/mun3im/mygardenbird.git
git clone https://github.com/mun3im/mynanet.git

# Train MynaNet on MyGardenBird
cd mynanet
python model/train.py --dataset ../mygardenbird/dataset

# Deploy to single microcontroller (M7 standalone)
python deployment/tflite/benchmark_tflite.py
```

**Result**: Standalone species classifier, assumes input already contains bird audio.

---

### Use Case 3: "I want the complete ARGUS dual-core system"

```bash
# Clone ARGUS with all submodules
git clone --recursive https://github.com/mun3im/argus.git
cd argus

# Everything is already linked via submodules:
# - models/mybadnet/ → mybad repo
# - models/mynanet/ → mynanet repo
# - datasets/mybad/ → mybad repo
# - datasets/mygardenbird/ → mygardenbird repo

# Deploy dual-core system to Portenta H7
cd deployment/portenta_h7
# Upload firmware following deployment guide
```

**Result**: Complete two-stage system with gatekeeper + classifier on dual cores.

---

### Use Case 4: "I want to benchmark models on my own hardware"

```bash
# Clone model repositories
git clone https://github.com/mun3im/mybad.git
git clone https://github.com/mun3im/mynanet.git

# Use pretrained models
# MyBADNet: mybad/mybadnet/pretrained/mybadnet_int8.tflite
# MynaNet: mynanet/pretrained/mynanet_int8.tflite

# Deploy to your target (e.g., ESP32-S3, Raspberry Pi Pico)
# Each repo includes deployment utilities
```

**Result**: Model weights without full system integration.

---

### Use Case 5: "I want to train a custom model on MyBAD"

```bash
# Clone dataset only
git clone https://github.com/mun3im/mybad.git
cd mybad

# Download data
python dataset/download_scripts/download_from_zenodo.py

# Implement custom model (not MyBADNet)
# Use dataset/splits/ for train/val/test
python my_custom_model.py --data dataset/splits/train.csv
```

**Result**: Use MyBAD dataset with your own architecture.

---

## README Cross-References

Each repository should link to related repos in its README:

### `mybad/README.md`
```markdown
## Related Repositories
- **MynaNet** (species classifier): [github.com/mun3im/mynanet](https://github.com/mun3im/mynanet)
- **MyGardenBird** (species dataset): [github.com/mun3im/mygardenbird](https://github.com/mun3im/mygardenbird)
- **ARGUS** (dual-core deployment): [github.com/mun3im/argus](https://github.com/mun3im/argus)
```

### `mygardenbird/README.md`
```markdown
## Related Repositories
- **MynaNet** (classification model): [github.com/mun3im/mynanet](https://github.com/mun3im/mynanet)
- **MyBAD** (activity detection): [github.com/mun3im/mybad](https://github.com/mun3im/mybad)
- **ARGUS** (dual-core deployment): [github.com/mun3im/argus](https://github.com/mun3im/argus)
```

### `mynanet/README.md`
```markdown
## Related Repositories
- **MyGardenBird** (training dataset): [github.com/mun3im/mygardenbird](https://github.com/mun3im/mygardenbird)
- **MyBAD** (activity detection): [github.com/mun3im/mybad](https://github.com/mun3im/mybad)
- **ARGUS** (edge deployment): [github.com/mun3im/argus](https://github.com/mun3im/argus)
```

### `argus/README.md`
```markdown
## Repository Structure
This is a multi-repository project using git submodules:
- **MyBAD** (dataset + MyBADNet): [github.com/mun3im/mybad](https://github.com/mun3im/mybad)
- **MyGardenBird** (dataset): [github.com/mun3im/mygardenbird](https://github.com/mun3im/mygardenbird)
- **MynaNet** (model): [github.com/mun3im/mynanet](https://github.com/mun3im/mynanet)
- **ARGUS** (this repo): System integration

Clone with `--recursive` to get all submodules automatically.
```

---

## Git Submodule Workflow

### Initial Setup (in `argus` repo)

```bash
cd argus

# Add submodules
git submodule add https://github.com/mun3im/mybad.git models/mybadnet
git submodule add https://github.com/mun3im/mynanet.git models/mynanet
git submodule add https://github.com/mun3im/mybad.git datasets/mybad
git submodule add https://github.com/mun3im/mygardenbird.git datasets/mygardenbird

git commit -m "Add model and dataset submodules"
git push
```

### User Cloning ARGUS

```bash
# Option 1: Clone with submodules immediately
git clone --recursive https://github.com/mun3im/argus.git

# Option 2: Clone first, then get submodules
git clone https://github.com/mun3im/argus.git
cd argus
git submodule update --init --recursive
```

### Updating Submodules

```bash
cd argus

# Update all submodules to latest commits
git submodule update --remote --merge

# Commit the updated submodule references
git add .
git commit -m "Update submodules to latest versions"
git push
```

---

## Paper Organization

### Paper 1: MyBAD Dataset Paper
- **Repository**: `mybad`
- **Includes**: Dataset description, curation process, MyBADNet baseline
- **Citation**: Points to `github.com/mun3im/mybad`

### Paper 2: MyGardenBird Dataset Paper (Optional)
- **Repository**: `mygardenbird`
- **Includes**: Species dataset description, MIP splitting
- **Citation**: Points to `github.com/mun3im/mygardenbird`

### Paper 3: MynaNet Model Paper (Optional)
- **Repository**: `mynanet`
- **Includes**: Architecture, training, ablation studies
- **Citation**: Points to `github.com/mun3im/mynanet`

### Paper 4: ARGUS System Paper (Main Contribution)
- **Repository**: `argus`
- **Includes**: Dual-core deployment, power optimization, integration
- **Citation**: Points to `github.com/mun3im/argus`
- **References**: Cites MyBAD, MyGardenBird, MyBADNet, MynaNet

---

## File Organization Summary

### What Goes Where?

| Content | mybad | mygardenbird | mynanet | argus |
|---------|-------|--------------|---------|-------|
| MyBAD dataset | ✅ | ❌ | ❌ | 🔗 submodule |
| MyGardenBird dataset | ❌ | ✅ | ❌ | 🔗 submodule |
| MyBADNet model | ✅ | ❌ | ❌ | 🔗 submodule |
| MynaNet model | ❌ | ❌ | ✅ | 🔗 submodule |
| Baseline experiments | ✅ | ❌ | ❌ | ❌ |
| Dual-core firmware | ❌ | ❌ | ❌ | ✅ |
| RPC/shared memory | ❌ | ❌ | ❌ | ✅ |
| Power profiling | ❌ | ❌ | ❌ | ✅ |
| ARGUS paper | ❌ | ❌ | ❌ | ✅ |

### File Count Estimates

| Repository | Files | Size | Purpose |
|------------|-------|------|---------|
| mybad | ~100-500 | ~50 MB | Dataset scripts + model |
| mygardenbird | ~50-100 | ~20 MB | Dataset scripts |
| mynanet | ~30-50 | ~10 MB | Model only |
| argus | ~50-100 | ~20 MB | Integration + paper |

---

## Common Pitfalls to Avoid

### ❌ Don't Do This

1. **Don't duplicate model implementations**
   - MyBADNet should only exist in `mybad` repo
   - ARGUS should use it via submodule, not copy code

2. **Don't put datasets in argus**
   - Datasets stay in their own repos
   - ARGUS links to them via submodules or download scripts

3. **Don't put deployment code in model repos**
   - Model repos: training, evaluation, quantization
   - ARGUS repo: hardware deployment, firmware, RPC

4. **Don't mix paper content**
   - MyBAD paper → `mybad` repo
   - ARGUS paper → `argus` repo
   - Don't duplicate LaTeX files

5. **Don't forget cross-references**
   - Every README should link to related repos
   - Users should easily discover the full ecosystem

### ✅ Do This

1. **Keep repos independently useful**
   - Each repo should work standalone
   - Users shouldn't be forced to clone everything

2. **Use git submodules for integration**
   - ARGUS repo pulls in components via submodules
   - Version control is clean and traceable

3. **Maintain clear READMEs**
   - Explain what's in this repo
   - Link to related repos
   - Provide quick start guides

4. **Keep pretrained weights accessible**
   - Include in repo if small (<100 MB)
   - Host on HuggingFace/Zenodo if large
   - Always provide download scripts

---

## Migration Checklist

### Phase 1: Organize Local Files
- [ ] Separate MyBADNet code from dataset curation
- [ ] Identify what belongs in each repo
- [ ] Clean up experimental code vs. production code

### Phase 2: Update `mybad` Repository
- [ ] Move MyBADNet implementation to `mybadnet/` folder
- [ ] Keep baseline experiments in `baselines/`
- [ ] Update README with structure from this document
- [ ] Add pretrained weights
- [ ] Tag release (e.g., v1.0.0)

### Phase 3: Update `mygardenbird` Repository
- [ ] Organize dataset files
- [ ] Add MIP split metadata
- [ ] Update README with species list
- [ ] Link to `mynanet` repo
- [ ] Tag release (e.g., v1.0.0)

### Phase 4: Update `mynanet` Repository
- [ ] Add model architecture code
- [ ] Add training scripts
- [ ] Include pretrained weights (433.9 KB INT8)
- [ ] Update README with performance metrics
- [ ] Tag release (e.g., v1.0.0)

### Phase 5: Setup `argus` Repository
- [ ] Create directory structure
- [ ] Add git submodules for mybad, mygardenbird, mynanet
- [ ] Add dual-core firmware (when ready)
- [ ] Add deployment guides
- [ ] Copy ARGUS paper to `docs/paper/`
- [ ] Update README with system overview
- [ ] Tag release when hardware deployment complete

### Phase 6: Cross-Reference All READMEs
- [ ] Add "Related Repositories" section to each README
- [ ] Verify all links work
- [ ] Add citation instructions
- [ ] Add license information

### Phase 7: Documentation
- [ ] Create this ARGUS_SITEMAP.md in each repo
- [ ] Add deployment guides
- [ ] Add troubleshooting sections
- [ ] Create contribution guidelines (if open to PRs)

---

## Version Control Strategy

### Tagging Releases

```bash
# In mybad repo
git tag -a v1.0.0 -m "MyBAD dataset v1.0 + MyBADNet baseline"
git push origin v1.0.0

# In mygardenbird repo
git tag -a v1.0.0 -m "MyGardenBird dataset v1.0"
git push origin v1.0.0

# In mynanet repo
git tag -a v1.0.0 -m "MynaNet model v1.0 (95.83% accuracy)"
git push origin v1.0.0

# In argus repo (after hardware deployment)
git tag -a v1.0.0 -m "ARGUS system v1.0 (dual-core Portenta H7)"
git push origin v1.0.0
```

### Synchronizing Versions

When ARGUS paper is published:
1. Tag all 4 repos with consistent version (e.g., v1.0.0)
2. Update ARGUS submodules to point to tagged versions
3. Create GitHub releases with DOI (via Zenodo)
4. Update paper with exact commit SHAs or DOIs

---

## Questions & Answers

### Q: Should MyBADNet be in `mybad` or `argus`?
**A**: `mybad`. It's the reference implementation for the dataset, useful independently of ARGUS.

### Q: What if someone wants to use MynaNet without MyBADNet?
**A**: Clone `mynanet` and `mygardenbird` only. No need for ARGUS or MyBAD.

### Q: What if someone wants just the datasets?
**A**: Clone `mybad` and/or `mygardenbird`. Models are in separate folders, easy to ignore.

### Q: How do I update a submodule in ARGUS?
**A**:
```bash
cd argus/models/mynanet
git pull origin main
cd ../../
git add models/mynanet
git commit -m "Update mynanet submodule"
git push
```

### Q: Can I use ARGUS with my own models?
**A**: Yes! Replace the submodules or modify deployment code to load your custom TFLite models.

### Q: Where should I report issues?
**A**:
- MyBAD dataset issues → `mybad` repo
- MynaNet model issues → `mynanet` repo
- Deployment/hardware issues → `argus` repo
- MyGardenBird dataset issues → `mygardenbird` repo

---

## Contact & Contributions

For questions about repository organization:
- Open an issue in the relevant repository
- Email: [your-email]
- Cite this sitemap when asking questions

---

**End of ARGUS Repository Sitemap**

This document should be copied to all 4 repositories for consistency.

Last updated: 2026-02-28
