#include "led_strip_func.h"
#include "led_strip.h"   // core driver (for *_core calls)

#include <string.h>
#include "esp_log.h"

#define TAG "led_strip_func"

// ==================================================
// Internal state
// ==================================================
static uint8_t g_brightness = 255; // full brightness

// ==================================================
// Helpers
// ==================================================
static inline rgb_t apply_brightness(rgb_t c)
{
    if (g_brightness == 255)
        return c;

    rgb_t out;
    out.r = (uint16_t)c.r * g_brightness / 255;
    out.g = (uint16_t)c.g * g_brightness / 255;
    out.b = (uint16_t)c.b * g_brightness / 255;
    return out;
}

// ==================================================
// Lifecycle (PUBLIC API)
// ==================================================
void led_strip_init(led_strip_t *strip)
{
    if (!strip)
        return;

    ESP_LOGI(TAG, "LED strip helper layer active");
}

void led_strip_free(led_strip_t *strip)
{
    if (!strip)
        return;

    led_strip_clear(strip);
    led_strip_free_core(strip);
}

// ==================================================
// Output helpers
// ==================================================
void led_strip_refresh(led_strip_t *strip)
{
    if (!strip)
        return;

    led_strip_refresh_core(strip);
}

void led_strip_clear(led_strip_t *strip)
{
    if (!strip)
        return;

    rgb_t black = { 0, 0, 0 };

    for (size_t i = 0; i < strip->length; i++)
        led_strip_set_pixel(strip, i, black);

    led_strip_refresh(strip);
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

    rgb_t scaled = apply_brightness(color);
    led_strip_set_pixel_core(strip, index, scaled);
}

void led_strip_fill(
    led_strip_t *strip,
    rgb_t color
)
{
    if (!strip)
        return;

    rgb_t scaled = apply_brightness(color);

    for (size_t i = 0; i < strip->length; i++)
        led_strip_set_pixel_core(strip, i, scaled);

    led_strip_refresh(strip);
}

// ==================================================
// Brightness control
// ==================================================
void led_strip_set_brightness(uint8_t level)
{
    g_brightness = level;
}

uint8_t led_strip_get_brightness(void)
{
    return g_brightness;
}