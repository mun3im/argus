# DrongoNet — Bird Activity Detector (Tier 1 Sentinel)

DrongoNet is a family of ultra-compact CNNs (<8 KB INT8) for binary bird/no-bird
detection. Three deployment variants are provided for different resource profiles.

## Variants

| Folder | Target | Model Size | Inference Time |
|--------|--------|-----------|----------------|
| `DrongoNet_Nano/` | Profiling baseline | ~4 KB | ~8 ms (M7) |
| `DrongoNet_Micro/` | Production Tier 1 (M4) | ~8 KB | ~40 ms (M4) |
| `DrongoNet_Edge/` | TFLite Micro edge build | ~8 KB | ~40 ms (M4) |

## Full Documentation

→ [Guide 03 — DrongoNet BAD CNN](./Guide_03_DrongoNet_BAD_CNN.md)
