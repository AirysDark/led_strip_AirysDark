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
    automatically gets helper functions
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
// Strip descriptor (shared between layers)
// ==================================================
typedef struct led_strip_t {
    led_strip_type_t      type;
    led_strip_order_t     order;
    uint8_t               brightness;   // 0–255 (handled by helper)
    size_t                length;
    gpio_num_t            gpio;

    // --- RMT core ---
    rmt_channel_handle_t  channel;
    rmt_encoder_handle_t  encoder;

    uint8_t              *buf;           // length * 3 bytes (RAW GRB)
} led_strip_t;

/* ==================================================
   CORE LAYER (PRIVATE – helper only)
   DO NOT call directly from app code
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
#endif    rmt_encoder_handle_t  encoder;

    uint8_t              *buf;
} led_strip_t;

/* ==================================================
   CORE (PRIVATE ? INTERNAL ONLY)
   ================================================== */

esp_err_t led_strip_core_init(led_strip_t *strip);
esp_err_t led_strip_core_free(led_strip_t *strip);
esp_err_t led_strip_core_refresh(led_strip_t *strip);
esp_err_t led_strip_core_set_pixel(led_strip_t *strip, size_t index, rgb_t color);

#ifdef __cplusplus
}
#endif
