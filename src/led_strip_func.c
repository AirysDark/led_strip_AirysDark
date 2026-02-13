#include "led_strip_func.h"
#include "led_strip.h"   // core layer access

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// ==================================================
// Internal state (global helper state)
// ==================================================
static uint8_t g_brightness = 255; // full brightness
static bool    g_gamma_enabled = false;
static bool    g_gamma_ready   = false;
static uint8_t g_gamma_lut[256];

// ==================================================
// Gamma LUT (Part 6)
// ==================================================
static void gamma_init(void)
{
    if (g_gamma_ready)
        return;

    for (int i = 0; i < 256; i++) {
        float x = (float)i / 255.0f;
        g_gamma_lut[i] = (uint8_t)(powf(x, 2.2f) * 255.0f + 0.5f);
    }

    g_gamma_ready = true;
}

static inline rgb_t apply_gamma(rgb_t c)
{
    if (!g_gamma_enabled)
        return c;

    gamma_init();
    c.r = g_gamma_lut[c.r];
    c.g = g_gamma_lut[c.g];
    c.b = g_gamma_lut[c.b];
    return c;
}

// ==================================================
// Helpers (existing + extended)
// ==================================================
static inline rgb_t scale_and_reorder(
    led_strip_t *strip,
    rgb_t c
)
{
    // ---- gamma ----
    c = apply_gamma(c);

    // ---- brightness ----
    if (g_brightness != 255) {
        c.r = (uint16_t)c.r * g_brightness / 255;
        c.g = (uint16_t)c.g * g_brightness / 255;
        c.b = (uint16_t)c.b * g_brightness / 255;
    }

    // ---- color order ----
    switch (strip->order) {
        case LED_ORDER_RGB:
            return (rgb_t){ c.r, c.g, c.b };

        case LED_ORDER_BRG:
            return (rgb_t){ c.b, c.r, c.g };

        case LED_ORDER_GRB:
        default:
            return (rgb_t){ c.g, c.r, c.b };
    }
}

// ==================================================
// Lifecycle (PUBLIC API)
// ==================================================
void led_strip_init(led_strip_t *strip)
{
    if (!strip)
        return;

    if (strip->order > LED_ORDER_BRG)
        strip->order = LED_ORDER_GRB;

    led_strip_core_init(strip);
}

void led_strip_free(led_strip_t *strip)
{
    if (!strip)
        return;

    led_strip_clear(strip);
    led_strip_core_free(strip);
}

// ==================================================
// Output helpers
// ==================================================
void led_strip_refresh(led_strip_t *strip)
{
    if (!strip)
        return;

    led_strip_core_refresh(strip);
}

void led_strip_clear(led_strip_t *strip)
{
    if (!strip)
        return;

    rgb_t black = { 0, 0, 0 };
    led_strip_fill(strip, black);
}

// ==================================================
// Part 7 ? Async refresh
// ==================================================
void led_strip_refresh_async(led_strip_t *strip)
{
    if (!strip)
        return;

    rmt_transmit_config_t cfg = {
        .loop_count = 0
    };

    rmt_transmit(
        strip->channel,
        strip->composite_encoder,
        strip->buf,
        strip->length * 3,
        &cfg
    );
}

bool led_strip_is_busy(led_strip_t *strip)
{
    if (!strip)
        return false;

    return rmt_tx_wait_all_done(strip->channel, 0) == ESP_ERR_TIMEOUT;
}

// ==================================================
// Pixel helpers
// ==================================================
void led_strip_set_pixel(
    led_strip_t *strip,
    size_t index,
    rgb_t color
)
{
    if (!strip || index >= strip->length)
        return;

    rgb_t mapped = scale_and_reorder(strip, color);
    led_strip_core_set_pixel(strip, index, mapped);
}

void led_strip_fill(
    led_strip_t *strip,
    rgb_t color
)
{
    if (!strip)
        return;

    rgb_t mapped = scale_and_reorder(strip, color);

    for (size_t i = 0; i < strip->length; i++)
        led_strip_core_set_pixel(strip, i, mapped);

    led_strip_core_refresh(strip);
}

// ==================================================
// Part 8 ? RGBW support (SK6812)
// ==================================================
void led_strip_set_pixel_rgbw(
    led_strip_t *strip,
    size_t index,
    rgbw_t color
)
{
    rgb_t mixed = {
        .r = color.r + color.w,
        .g = color.g + color.w,
        .b = color.b + color.w
    };

    led_strip_set_pixel(strip, index, mixed);
}

// ==================================================
// Brightness + Gamma control
// ==================================================
void led_strip_set_brightness(uint8_t level)
{
    g_brightness = level;
}

uint8_t led_strip_get_brightness(void)
{
    return g_brightness;
}

void led_strip_enable_gamma(bool enable)
{
    g_gamma_enabled = enable;
}