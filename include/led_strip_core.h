#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_err.h"
#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
    LED STRIP CORE (PRIVATE)

    - Hardware-only RMT TX driver
    - No brightness logic
    - No color order logic
    - No public API exposure
*/

/* Forward declaration only */
typedef struct led_strip_t led_strip_t;

/* -------------------------------------------------
   Core lifecycle
--------------------------------------------------*/
esp_err_t led_strip_core_init(led_strip_t *strip);
esp_err_t led_strip_core_free(led_strip_t *strip);

/* -------------------------------------------------
   Core output
--------------------------------------------------*/
esp_err_t led_strip_core_refresh(led_strip_t *strip);

/* === PART 7: ASYNC SUPPORT === */
esp_err_t led_strip_core_refresh_async(led_strip_t *strip);
bool      led_strip_core_is_busy(led_strip_t *strip);

/* -------------------------------------------------
   Core pixel write (RAW BYTES)
--------------------------------------------------*/
esp_err_t led_strip_core_set_pixel(
    led_strip_t *strip,
    size_t index,
    rgb_t color
);

#ifdef __cplusplus
}
#endif