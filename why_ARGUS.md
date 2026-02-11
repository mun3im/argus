# ARGUS

## Autonomous Real-time Guardian for Ultra-low-power Species Monitoring

------------------------------------------------------------------------

## Overview

**ARGUS** is a two-tier ultra-low-power biologging system designed for
long-term autonomous bird monitoring in tropical rainforest
environments.

Inspired by vigilance in nature and mythology, ARGUS combines an
always-on **gatekeeper** with an on-demand **classifier**, ensuring high
recall of biological events while maintaining multi-year battery life.

------------------------------------------------------------------------

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

------------------------------------------------------------------------

## Why the Name "ARGUS"?

ARGUS represents three aligned dimensions:

### 1. Ornithological

**Great Argus (Argusianus argus)**\
- Native to Peninsular Malaysia, Sumatra, and Borneo\
- Males display 100+ eye-like ocelli on their wings\
- IUCN Near Threatened (habitat loss)

In Malay, it is known as *Kuang Raja* (King Pheasant).\
Its "Bulu Seribu Mata" (thousand-eyed feathers) symbolize vigilance.

### 2. Mythological

**Argus Panoptes** -- the hundred-eyed Greek guardian who never slept.\
Like the mythological Argus, the system never fully sleeps --- not all
eyes close at once.

### 3. Engineering

**A**utonomous\
**R**eal-time\
**G**uardian\
for\
**U**ltra-low-power\
**S**pecies monitoring

The acronym directly reflects the system's architectural philosophy.

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

------------------------------------------------------------------------

## License

MIT

------------------------------------------------------------------------

## Citation

If you use ARGUS in academic work, please cite the associated
publication (to be added).
