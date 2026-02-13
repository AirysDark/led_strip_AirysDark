#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration only (core struct lives in led_strip.h)
typedef struct led_strip_t led_strip_t;

// ==================================================
// Public helper API (THIS is what app code uses)
// ==================================================

void led_strip_init(led_strip_t *strip);
void led_strip_free(led_strip_t *strip);

void led_strip_refresh(led_strip_t *strip);
void led_strip_clear(led_strip_t *strip);

void led_strip_set_pixel(
    led_strip_t *strip,
    size_t index,
    rgb_t color
);

void led_strip_fill(
    led_strip_t *strip,
    rgb_t color
);

// ==================================================
// Brightness (software scaling)
// ==================================================
void    led_strip_set_brightness(uint8_t level); // 0?255
uint8_t led_strip_get_brightness(void);

// ==================================================
// Part 6 ? Gamma correction
// ==================================================
void led_strip_enable_gamma(bool enable);

// ==================================================
// Part 7 ? Async / non-blocking refresh
// ==================================================
void led_strip_refresh_async(led_strip_t *strip);
bool led_strip_is_busy(led_strip_t *strip);

// ==================================================
// Part 8 ? RGBW support
// ==================================================
void led_strip_set_pixel_rgbw(
    led_strip_t *strip,
    size_t index,
    rgbw_t color
);

#ifdef __cplusplus
}
#endif