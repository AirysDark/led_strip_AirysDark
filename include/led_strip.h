#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"
#include "color.h"

// PUBLIC API comes from helper layer
#include "led_strip_func.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LED_STRIP_WS2812 = 0,
} led_strip_type_t;

typedef enum {
    LED_ORDER_GRB = 0,
    LED_ORDER_RGB,
    LED_ORDER_BRG
} led_strip_order_t;

typedef struct led_strip_t {
    led_strip_type_t      type;
    led_strip_order_t     order;
    uint8_t               brightness;
    size_t                length;
    gpio_num_t            gpio;

    rmt_channel_handle_t  channel;
    rmt_encoder_handle_t  encoder;

    uint8_t              *buf;
} led_strip_t;

/* =========================
   CORE (PRIVATE ? DO NOT USE)
   ========================= */

esp_err_t led_strip_core_init(led_strip_t *strip);
esp_err_t led_strip_core_free(led_strip_t *strip);
esp_err_t led_strip_core_refresh(led_strip_t *strip);
esp_err_t led_strip_core_set_pixel(led_strip_t *strip, size_t index, rgb_t color);

#ifdef __cplusplus
}
#endif