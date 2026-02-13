#include "led_strip_func.h"

#include "led_strip.h"   // Implementation is allowed to include this
#include <string.h>
#include "esp_log.h"

#define TAG "led_strip_func"

// --------------------------------------------------
// Internal state
// --------------------------------------------------

static uint8_t g_brightness = 255; // full brightness

// --------------------------------------------------
// Helpers
// --------------------------------------------------

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

// --------------------------------------------------
// Lifecycle
// --------------------------------------------------

void led_strip_func_init(led_strip_t *strip)
{
    if (!strip)
        return;

    ESP_LOGI(TAG, "Helper layer ready");
}

void led_strip_func_deinit(led_strip_t *strip)
{
    if (!strip)
        return;

    led_strip_func_clear(strip);
}

// --------------------------------------------------
// Output helpers
// --------------------------------------------------

void led_strip_func_flush(led_strip_t *strip)
{
    if (!strip)
        return;

    led_strip_flush(strip);
}

void led_strip_func_clear(led_strip_t *strip)
{
    if (!strip)
        return;

    rgb_t black = { 0, 0, 0 };

    for (size_t i = 0; i < strip->length; i++)
        led_strip_set_pixel(strip, i, black);

    led_strip_flush(strip);
}

// --------------------------------------------------
// Pixel helpers
// --------------------------------------------------

void led_strip_func_set_pixel(
    led_strip_t *strip,
    size_t index,
    rgb_t color
)
{
    if (!strip || index >= strip->length)
        return;

    rgb_t scaled = apply_brightness(color);
    led_strip_set_pixel(strip, index, scaled);
}

void led_strip_func_fill(
    led_strip_t *strip,
    rgb_t color
)
{
    if (!strip)
        return;

    rgb_t scaled = apply_brightness(color);

    for (size_t i = 0; i < strip->length; i++)
        led_strip_set_pixel(strip, i, scaled);

    led_strip_flush(strip);
}

// --------------------------------------------------
// Brightness control
// --------------------------------------------------

void led_strip_func_set_brightness(uint8_t level)
{
    g_brightness = level;
}

uint8_t led_strip_func_get_brightness(void)
{
    return g_brightness;
}