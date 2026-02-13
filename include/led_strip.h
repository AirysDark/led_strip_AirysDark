#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"
#include "color.h"

/*
    PUBLIC API = helper layer

    Any file that includes led_strip.h
    automatically gets the helper functions
    implemented in led_strip_func.c
*/
#include "led_strip_func.h"

#ifdef __cplusplus
extern "C" {
#endif

// ==================================================
// LED type (future-proof)
// ==================================================
typedef enum {
    LED_STRIP_WS2812 = 0,
} led_strip_type_t;

// ==================================================
// RGB byte order
// ==================================================
typedef enum {
    LED_ORDER_GRB = 0,
    LED_ORDER_RGB,
    LED_ORDER_BRG
} led_strip_order_t;

// ==================================================
// Strip descriptor (shared between core + helper)
// ==================================================
typedef struct led_strip_t {
    // Logical configuration
    led_strip_type_t      type;
    led_strip_order_t     order;

    // Brightness (0?255), applied by helper layer
    uint8_t               brightness;

    size_t                length;
    gpio_num_t            gpio;

    // --- RMT core handles ---
    rmt_channel_handle_t  channel;
    rmt_encoder_handle_t  bytes_encoder;
    rmt_encoder_handle_t  reset_encoder;
    rmt_encoder_handle_t  composite_encoder;

    // Raw pixel buffer (GRB, unscaled)
    uint8_t              *buf;   // length * 3 bytes
} led_strip_t;

/* ==================================================
   CORE (INTERNAL ONLY)
   Helper layer calls these.
   User code MUST NOT call them directly.
   ================================================== */

esp_err_t led_strip_core_init(led_strip_t *strip);
esp_err_t led_strip_core_free(led_strip_t *strip);
esp_err_t led_strip_core_refresh(led_strip_t *strip);
esp_err_t led_strip_core_set_pixel(
    led_strip_t *strip,
    size_t index,
    rgb_t color
);

#ifdef __cplusplus
}
#endif