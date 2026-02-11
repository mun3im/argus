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

| Dimension        | Reference | Key Points | Symbolic Meaning for the System |
|------------------|------------|------------|----------------------------------|
| **Ornithological** | **Great Argus (*Argusianus argus*)** | • Native to Peninsular Malaysia, Sumatra, Borneo  <br>• Males display 100+ eye-like ocelli  <br>• IUCN Near Threatened (habitat loss)  <br>• Known in Malay as *Kuang Raja* (King Pheasant)  <br>• "Bulu Seribu Mata" (thousand-eyed feathers) | Native ecological anchor  <br>Embodiment of vigilance  <br>Conservation relevance |
| **Mythological** | **Argus Panoptes** | • Hundred-eyed Greek guardian  <br>• Never fully slept | Perpetual vigilance  <br>Tiered architecture — not all “eyes” close at once |
| **Engineering** | **ARGUS Acronym** | **A**utonomous  <br>**R**eal-time  <br>**G**uardian  <br>for  <br>**U**ltra-low-power  <br>**S**pecies monitoring | Direct architectural description  <br>Gatekeeper-centric philosophy |

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
