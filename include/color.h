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
// Predefined colors (stored in flash, defined in color.c)
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
// Luma helper (used for RGBW or brightness math)
// --------------------------------------------------
static inline uint8_t rgb_luma(rgb_t c)
{
    return (uint8_t)(
        ((uint16_t)c.r * 30 +
         (uint16_t)c.g * 59 +
         (uint16_t)c.b * 11) / 100
    );
}

#ifdef __cplusplus
}
#endif
