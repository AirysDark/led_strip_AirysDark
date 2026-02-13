#include "led_strip_func.h"
#include "led_strip.h"

static uint8_t g_brightness = 255;

static rgb_t scale(rgb_t c)
{
    c.r = (c.r * g_brightness) / 255;
    c.g = (c.g * g_brightness) / 255;
    c.b = (c.b * g_brightness) / 255;
    return c;
}

void led_strip_init(led_strip_t *strip)
{
    if (!strip) return;
    led_strip_core_init(strip);
}

void led_strip_free(led_strip_t *strip)
{
    if (!strip) return;
    led_strip_clear(strip);
    led_strip_core_free(strip);
}

void led_strip_refresh(led_strip_t *strip)
{
    led_strip_core_refresh(strip);
}

void led_strip_clear(led_strip_t *strip)
{
    rgb_t black = {0,0,0};
    led_strip_fill(strip, black);
}

void led_strip_set_pixel(led_strip_t *strip, size_t index, rgb_t color)
{
    led_strip_core_set_pixel(strip, index, scale(color));
}

void led_strip_fill(led_strip_t *strip, rgb_t color)
{
    for (size_t i = 0; i < strip->length; i++)
        led_strip_set_pixel(strip, i, color);

    led_strip_refresh(strip);
}

void led_strip_set_brightness(uint8_t level)
{
    g_brightness = level;
}

uint8_t led_strip_get_brightness(void)
{
    return g_brightness;
}