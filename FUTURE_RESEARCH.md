# ARGUS System Enhancement Roadmap

This document outlines technical upgrades that can strengthen ARGUS in
robustness, biological fidelity, and long-term autonomy.

------------------------------------------------------------------------

## 1. Overlapping Windows in Tier 1 (High Priority)

### Problem

Fixed 3-second windows create temporal blind spots. Short calls at
window boundaries may be partially captured and missed.

### Proposed Solution

-   Use 50% overlap
-   Wake every 1.5 seconds
-   Analyze 3-second windows with shared buffer

### Benefit

-   Reduces boundary-related false negatives
-   Improves true biological recall

------------------------------------------------------------------------

## 2. Adaptive Noise Floor Calibration

### Problem

Environmental noise varies over time (rain, wind, insects, rivers).
Static thresholds drift.

### Proposed Solution

-   Maintain rolling 5-minute median RMS
-   Adjust detection threshold relative to noise baseline

### Benefit

-   Reduces false triggers during heavy noise
-   Maintains sensitivity during quiet periods

------------------------------------------------------------------------

## 3. On-Device Hard Negative Mining

### Concept

If Tier 1 triggers but Tier 2 confidence is low or "no species," save
clip as hard negative.

### Benefit

-   Captures real environmental confusers
-   Improves future gatekeeper retraining
-   Enables domain adaptation from field deployments

------------------------------------------------------------------------

## 4. Confidence-Aware Tier 2 Inference

### Enhancement

-   Use Monte Carlo Dropout (3 passes)
-   Measure prediction entropy/variance
-   Save ambiguous samples

### Benefit

-   Prevents silent misclassification
-   Supports long-term dataset expansion

------------------------------------------------------------------------

## 5. Biological Event Grouping

### Concept

Merge consecutive detections within a time threshold into a single
biological event.

### Benefit

-   Cleaner ecological metadata
-   Reduced SD write frequency
-   Better alignment with real vocalization patterns

------------------------------------------------------------------------

## 6. Dawn/Dusk Adaptive Sampling

### Concept

Increase sensitivity and wake frequency during peak vocalization
periods.

### Benefit

-   Ecologically aligned sampling
-   Better energy allocation
-   Improved detection during high-activity windows

------------------------------------------------------------------------

## 7. Redundant Detection Path

### Enhancement

Combine CNN-based detection with simple spectral or entropy-based
heuristic.

### Benefit

-   Increased robustness
-   Protection against domain shift or model degradation

------------------------------------------------------------------------

## 8. Self-Diagnostics and Drift Monitoring

### Add Logging Of:

-   Tier 1 trigger rate
-   Tier 2 confidence statistics
-   Background RMS trends

### Benefit

-   Detects mic failure
-   Identifies environmental shifts
-   Improves long-term autonomy

------------------------------------------------------------------------

## 9. Battery-Aware Adaptive Policy

### Concept

If battery voltage drops: - Increase detection threshold - Reduce wake
frequency - Switch to detection-only mode

### Benefit

-   Graceful performance degradation
-   Extends mission lifetime

------------------------------------------------------------------------

## Strategic Priority

If only one enhancement is implemented:

**Overlapping windows + pre-trigger buffering**

Reason: Recall integrity defines the value of ARGUS. Missing biological
events undermines the entire system.

------------------------------------------------------------------------

This roadmap supports the evolution of ARGUS from a working prototype
into a resilient, long-duration ecological monitoring platform.
