#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------
// RGB color structure
// --------------------------------------------------
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;

// --------------------------------------------------
// RGBW color structure (Part 8)
// --------------------------------------------------
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
} rgbw_t;

// --------------------------------------------------
// Predefined colors (flash-resident, defined in color.c)
// --------------------------------------------------
extern const rgb_t COLOR_RED;
extern const rgb_t COLOR_GREEN;
extern const rgb_t COLOR_BLUE;
extern const rgb_t COLOR_WHITE;
extern const rgb_t COLOR_BLACK;
extern const rgb_t COLOR_YELLOW;
extern const rgb_t COLOR_CYAN;
extern const rgb_t COLOR_MAGENTA;

// --------------------------------------------------
// Luma helper
// - Used for brightness math or RGBW derivation
// - Inline, zero overhead
// --------------------------------------------------
static inline uint8_t rgb_luma(rgb_t c)
{
    return (uint8_t)(
        ((uint16_t)c.r * 30 +
         (uint16_t)c.g * 59 +
         (uint16_t)c.b * 11) / 100
    );
}

// --------------------------------------------------
// RGBW helpers (Part 8)
// --------------------------------------------------

// Extract white component from RGB (simple model)
static inline uint8_t rgb_extract_white(rgb_t c)
{
    uint8_t w = c.r;
    if (c.g < w) w = c.g;
    if (c.b < w) w = c.b;
    return w;
}

// Convert RGB ? RGBW
static inline rgbw_t rgb_to_rgbw(rgb_t c)
{
    rgbw_t out;
    out.w = rgb_extract_white(c);
    out.r = c.r - out.w;
    out.g = c.g - out.w;
    out.b = c.b - out.w;
    return out;
}

// Convert RGBW ? RGB (for non-RGBW strips)
static inline rgb_t rgbw_to_rgb(rgbw_t c)
{
    rgb_t out;
    out.r = c.r + c.w;
    out.g = c.g + c.w;
    out.b = c.b + c.w;
    return out;
}

// --------------------------------------------------
// Gamma helpers (Part 6)
// --------------------------------------------------

// Apply gamma correction using lookup table
// LUT is provided by helper layer
static inline uint8_t gamma_apply(uint8_t v, const uint8_t *lut)
{
    return lut ? lut[v] : v;
}

static inline rgb_t rgb_gamma(rgb_t c, const uint8_t *lut)
{
    rgb_t out;
    out.r = gamma_apply(c.r, lut);
    out.g = gamma_apply(c.g, lut);
    out.b = gamma_apply(c.b, lut);
    return out;
}

#ifdef __cplusplus
}
#endif