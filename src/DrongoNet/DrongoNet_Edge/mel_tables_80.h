// mel_tables.h — Auto-generated mel filterbank tables for Portenta H7
// Generated: 2026-06-11 16:07:39
// Parameters: sr=16000, n_fft=1024, n_mels=80, fmin=100.0, fmax=8000.0
//
// Matches: librosa.filters.mel(sr=16000, n_fft=1024, n_mels=80, fmin=100.0, fmax=8000.0)
// Used in: seabadnet-edge training (80-mel, 184 frames)
//
// DO NOT EDIT — regenerate with: python generate_mel_tables.py

#ifndef MEL_TABLES_80_H
#define MEL_TABLES_80_H

// ---- Mel Spectrogram Constants ----
#define MEL_N_FFT       1024
#define MEL_HOP_LENGTH  256
#define MEL_N_FFT_BINS  (1024 / 2 + 1)  // 513
#define MEL_N_MELS      80
#define MEL_FMIN        100.0f
#define MEL_FMAX        8000.0f
#define MEL_SR          16000

// ---- Triangular Mel Filter Definition ----
// Each filter is a triangle: ramp up from startBin to peakBin,
//                            ramp down from peakBin to endBin.
// Weight at startBin = 0, at peakBin = 1, at endBin = 0.
// Runtime weight computation:
//   Rising:  w = (k - start) / (peak - start)
//   Falling: w = (end - k) / (end - peak)

typedef struct {
    int startBin;  // First non-zero FFT bin (inclusive)
    int peakBin;   // Peak of triangular filter
    int endBin;    // Last non-zero FFT bin (inclusive)
} MelFilterDef;

static const MelFilterDef melFilters[MEL_N_MELS] = {
    {   6,    7,    9},  // Filter  0:   100.0 –   123.9 –   148.6 Hz  (width=3 bins)
    {   7,    9,   11},  // Filter  1:   123.9 –   148.6 –   173.9 Hz  (width=4 bins)
    {   9,   11,   12},  // Filter  2:   148.6 –   173.9 –   200.1 Hz  (width=3 bins)
    {  11,   12,   14},  // Filter  3:   173.9 –   200.1 –   227.0 Hz  (width=3 bins)
    {  12,   14,   16},  // Filter  4:   200.1 –   227.0 –   254.7 Hz  (width=4 bins)
    {  14,   16,   18},  // Filter  5:   227.0 –   254.7 –   283.2 Hz  (width=4 bins)
    {  16,   18,   20},  // Filter  6:   254.7 –   283.2 –   312.6 Hz  (width=4 bins)
    {  18,   20,   21},  // Filter  7:   283.2 –   312.6 –   342.9 Hz  (width=3 bins)
    {  20,   21,   23},  // Filter  8:   312.6 –   342.9 –   374.1 Hz  (width=3 bins)
    {  21,   23,   26},  // Filter  9:   342.9 –   374.1 –   406.2 Hz  (width=5 bins)
    {  23,   26,   28},  // Filter 10:   374.1 –   406.2 –   439.3 Hz  (width=5 bins)
    {  26,   28,   30},  // Filter 11:   406.2 –   439.3 –   473.4 Hz  (width=4 bins)
    {  28,   30,   32},  // Filter 12:   439.3 –   473.4 –   508.4 Hz  (width=4 bins)
    {  30,   32,   34},  // Filter 13:   473.4 –   508.4 –   544.6 Hz  (width=4 bins)
    {  32,   34,   37},  // Filter 14:   508.4 –   544.6 –   581.8 Hz  (width=5 bins)
    {  34,   37,   39},  // Filter 15:   544.6 –   581.8 –   620.1 Hz  (width=5 bins)
    {  37,   39,   42},  // Filter 16:   581.8 –   620.1 –   659.6 Hz  (width=5 bins)
    {  39,   42,   44},  // Filter 17:   620.1 –   659.6 –   700.2 Hz  (width=5 bins)
    {  42,   44,   47},  // Filter 18:   659.6 –   700.2 –   742.1 Hz  (width=5 bins)
    {  44,   47,   50},  // Filter 19:   700.2 –   742.1 –   785.2 Hz  (width=6 bins)
    {  47,   50,   53},  // Filter 20:   742.1 –   785.2 –   829.6 Hz  (width=6 bins)
    {  50,   53,   56},  // Filter 21:   785.2 –   829.6 –   875.4 Hz  (width=6 bins)
    {  53,   56,   59},  // Filter 22:   829.6 –   875.4 –   922.5 Hz  (width=6 bins)
    {  56,   59,   62},  // Filter 23:   875.4 –   922.5 –   971.0 Hz  (width=6 bins)
    {  59,   62,   65},  // Filter 24:   922.5 –   971.0 –  1021.0 Hz  (width=6 bins)
    {  62,   65,   68},  // Filter 25:   971.0 –  1021.0 –  1072.4 Hz  (width=6 bins)
    {  65,   68,   72},  // Filter 26:  1021.0 –  1072.4 –  1125.4 Hz  (width=7 bins)
    {  68,   72,   75},  // Filter 27:  1072.4 –  1125.4 –  1180.0 Hz  (width=7 bins)
    {  72,   75,   79},  // Filter 28:  1125.4 –  1180.0 –  1236.2 Hz  (width=7 bins)
    {  75,   79,   82},  // Filter 29:  1180.0 –  1236.2 –  1294.1 Hz  (width=7 bins)
    {  79,   82,   86},  // Filter 30:  1236.2 –  1294.1 –  1353.7 Hz  (width=7 bins)
    {  82,   86,   90},  // Filter 31:  1294.1 –  1353.7 –  1415.1 Hz  (width=8 bins)
    {  86,   90,   94},  // Filter 32:  1353.7 –  1415.1 –  1478.4 Hz  (width=8 bins)
    {  90,   94,   98},  // Filter 33:  1415.1 –  1478.4 –  1543.5 Hz  (width=8 bins)
    {  94,   98,  103},  // Filter 34:  1478.4 –  1543.5 –  1610.6 Hz  (width=9 bins)
    {  98,  103,  107},  // Filter 35:  1543.5 –  1610.6 –  1679.7 Hz  (width=9 bins)
    { 103,  107,  112},  // Filter 36:  1610.6 –  1679.7 –  1750.8 Hz  (width=9 bins)
    { 107,  112,  116},  // Filter 37:  1679.7 –  1750.8 –  1824.1 Hz  (width=9 bins)
    { 112,  116,  121},  // Filter 38:  1750.8 –  1824.1 –  1899.6 Hz  (width=9 bins)
    { 116,  121,  126},  // Filter 39:  1824.1 –  1899.6 –  1977.3 Hz  (width=10 bins)
    { 121,  126,  131},  // Filter 40:  1899.6 –  1977.3 –  2057.4 Hz  (width=10 bins)
    { 126,  131,  137},  // Filter 41:  1977.3 –  2057.4 –  2139.8 Hz  (width=11 bins)
    { 131,  137,  142},  // Filter 42:  2057.4 –  2139.8 –  2224.7 Hz  (width=11 bins)
    { 137,  142,  148},  // Filter 43:  2139.8 –  2224.7 –  2312.2 Hz  (width=11 bins)
    { 142,  148,  153},  // Filter 44:  2224.7 –  2312.2 –  2402.3 Hz  (width=11 bins)
    { 148,  153,  159},  // Filter 45:  2312.2 –  2402.3 –  2495.0 Hz  (width=11 bins)
    { 153,  159,  165},  // Filter 46:  2402.3 –  2495.0 –  2590.6 Hz  (width=12 bins)
    { 159,  165,  172},  // Filter 47:  2495.0 –  2590.6 –  2689.0 Hz  (width=13 bins)
    { 165,  172,  178},  // Filter 48:  2590.6 –  2689.0 –  2790.3 Hz  (width=13 bins)
    { 172,  178,  185},  // Filter 49:  2689.0 –  2790.3 –  2894.6 Hz  (width=13 bins)
    { 178,  185,  192},  // Filter 50:  2790.3 –  2894.6 –  3002.1 Hz  (width=14 bins)
    { 185,  192,  199},  // Filter 51:  2894.6 –  3002.1 –  3112.8 Hz  (width=14 bins)
    { 192,  199,  206},  // Filter 52:  3002.1 –  3112.8 –  3226.8 Hz  (width=14 bins)
    { 199,  206,  214},  // Filter 53:  3112.8 –  3226.8 –  3344.3 Hz  (width=15 bins)
    { 206,  214,  221},  // Filter 54:  3226.8 –  3344.3 –  3465.2 Hz  (width=15 bins)
    { 214,  221,  229},  // Filter 55:  3344.3 –  3465.2 –  3589.7 Hz  (width=15 bins)
    { 221,  229,  238},  // Filter 56:  3465.2 –  3589.7 –  3718.0 Hz  (width=17 bins)
    { 229,  238,  246},  // Filter 57:  3589.7 –  3718.0 –  3850.1 Hz  (width=17 bins)
    { 238,  246,  255},  // Filter 58:  3718.0 –  3850.1 –  3986.1 Hz  (width=17 bins)
    { 246,  255,  264},  // Filter 59:  3850.1 –  3986.1 –  4126.3 Hz  (width=18 bins)
    { 255,  264,  273},  // Filter 60:  3986.1 –  4126.3 –  4270.6 Hz  (width=18 bins)
    { 264,  273,  283},  // Filter 61:  4126.3 –  4270.6 –  4419.2 Hz  (width=19 bins)
    { 273,  283,  292},  // Filter 62:  4270.6 –  4419.2 –  4572.3 Hz  (width=19 bins)
    { 283,  292,  303},  // Filter 63:  4419.2 –  4572.3 –  4729.9 Hz  (width=20 bins)
    { 292,  303,  313},  // Filter 64:  4572.3 –  4729.9 –  4892.3 Hz  (width=21 bins)
    { 303,  313,  324},  // Filter 65:  4729.9 –  4892.3 –  5059.5 Hz  (width=21 bins)
    { 313,  324,  335},  // Filter 66:  4892.3 –  5059.5 –  5231.7 Hz  (width=22 bins)
    { 324,  335,  346},  // Filter 67:  5059.5 –  5231.7 –  5409.1 Hz  (width=22 bins)
    { 335,  346,  358},  // Filter 68:  5231.7 –  5409.1 –  5591.7 Hz  (width=23 bins)
    { 346,  358,  370},  // Filter 69:  5409.1 –  5591.7 –  5779.9 Hz  (width=24 bins)
    { 358,  370,  382},  // Filter 70:  5591.7 –  5779.9 –  5973.6 Hz  (width=24 bins)
    { 370,  382,  395},  // Filter 71:  5779.9 –  5973.6 –  6173.2 Hz  (width=25 bins)
    { 382,  395,  408},  // Filter 72:  5973.6 –  6173.2 –  6378.7 Hz  (width=26 bins)
    { 395,  408,  422},  // Filter 73:  6173.2 –  6378.7 –  6590.3 Hz  (width=27 bins)
    { 408,  422,  436},  // Filter 74:  6378.7 –  6590.3 –  6808.3 Hz  (width=28 bins)
    { 422,  436,  450},  // Filter 75:  6590.3 –  6808.3 –  7032.8 Hz  (width=28 bins)
    { 436,  450,  465},  // Filter 76:  6808.3 –  7032.8 –  7264.0 Hz  (width=29 bins)
    { 450,  465,  480},  // Filter 77:  7032.8 –  7264.0 –  7502.2 Hz  (width=30 bins)
    { 465,  480,  496},  // Filter 78:  7264.0 –  7502.2 –  7747.4 Hz  (width=31 bins)
    { 480,  496,  512},  // Filter 79:  7502.2 –  7747.4 –  8000.0 Hz  (width=32 bins)
};

// Raw bin edge points (82 points for 80 filters):
// bins = [6, 7, 9, 11, 12, 14, 16, 18, 20, 21, 23, 26, 28, 30, 32, 34, 37, 39, 42, 44, 47, 50, 53, 56, 59, 62, 65, 68, 72, 75, 79, 82, 86, 90, 94, 98, 103, 107, 112, 116, 121, 126, 131, 137, 142, 148, 153, 159, 165, 172, 178, 185, 192, 199, 206, 214, 221, 229, 238, 246, 255, 264, 273, 283, 292, 303, 313, 324, 335, 346, 358, 370, 382, 395, 408, 422, 436, 450, 465, 480, 496, 512]
// Hz   = [100.0, 123.9, 148.6, 173.9, 200.1, 227.0, 254.7, 283.2, 312.6, 342.9, 374.1, 406.2, 439.3, 473.4, 508.4, 544.6, 581.8, 620.1, 659.6, 700.2, 742.1, 785.2, 829.6, 875.4, 922.5, 971.0, 1021.0, 1072.4, 1125.4, 1180.0, 1236.2, 1294.1, 1353.7, 1415.1, 1478.4, 1543.5, 1610.6, 1679.7, 1750.8, 1824.1, 1899.6, 1977.3, 2057.4, 2139.8, 2224.7, 2312.2, 2402.3, 2495.0, 2590.6, 2689.0, 2790.3, 2894.6, 3002.1, 3112.8, 3226.8, 3344.3, 3465.2, 3589.7, 3718.0, 3850.1, 3986.1, 4126.3, 4270.6, 4419.2, 4572.3, 4729.9, 4892.3, 5059.5, 5231.7, 5409.1, 5591.7, 5779.9, 5973.6, 6173.2, 6378.7, 6590.3, 6808.3, 7032.8, 7264.0, 7502.2, 7747.4, 8000.0]

#endif // MEL_TABLES_80_H
