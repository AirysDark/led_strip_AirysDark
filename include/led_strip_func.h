#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    High-level helpers for led_strip_AirysDark

    - Safe wrappers around core driver
    - Centralized flush / clear logic
    - Brightness scaling support
*/

// Forward declaration (NO include!)
typedef struct led_strip_t led_strip_t;

// --------------------------------------------------
// Lifecycle
// --------------------------------------------------

void led_strip_init(led_strip_t *strip);
void led_strip_deinit(led_strip_t *strip);

// --------------------------------------------------
// Output helpers
// --------------------------------------------------

void led_strip_flush(led_strip_t *strip);
void led_strip_clear(led_strip_t *strip);

// --------------------------------------------------
// Pixel helpers
// --------------------------------------------------

void led_strip_set_pixel(
    led_strip_t *strip,
    size_t index,
    rgb_t color
);

void led_strip_fill(
    led_strip_t *strip,
    rgb_t color
);

// --------------------------------------------------
// Brightness (software scaling)
// --------------------------------------------------

void led_strip_set_brightness(uint8_t level); // 0?255
uint8_t led_strip_get_brightness(void);

#ifdef __cplusplus
}
#endif