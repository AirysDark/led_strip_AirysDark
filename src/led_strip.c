#include "led_strip.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_rom_sys.h"

#define TAG "led_strip"

// ==================================================
// WS2812 timing (nanoseconds)
// ==================================================
#define T0H_NS   400
#define T0L_NS   850
#define T1H_NS   800
#define T1L_NS   450
#define RESET_US 60

#define NS_TO_TICKS(ns) ((ns) / 100)

#define CHECK(x)     do { esp_err_t r = (x); if (r != ESP_OK) return r; } while (0)
#define CHECK_ARG(x) do { if (!(x)) return ESP_ERR_INVALID_ARG; } while (0)

// ==================================================
// INIT (hardware + helper)
// ==================================================
esp_err_t led_strip_init(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->length > 0);

    strip->buf = calloc(strip->length * 3, 1);
    if (!strip->buf)
        return ESP_ERR_NO_MEM;

    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = strip->gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };

    CHECK(rmt_new_tx_channel(&tx_cfg, &strip->channel));

    rmt_bytes_encoder_config_t enc_cfg = {
        .bit0 = {
            .level0 = 1,
            .duration0 = NS_TO_TICKS(T0H_NS),
            .level1 = 0,
            .duration1 = NS_TO_TICKS(T0L_NS),
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = NS_TO_TICKS(T1H_NS),
            .level1 = 0,
            .duration1 = NS_TO_TICKS(T1L_NS),
        },
        .flags.msb_first = true,
    };

    CHECK(rmt_new_bytes_encoder(&enc_cfg, &strip->encoder));
    CHECK(rmt_enable(strip->channel));

    // defaults
    if (strip->brightness == 0)
        strip->brightness = 255;

    if (strip->order > LED_ORDER_BRG)
        strip->order = LED_ORDER_GRB;

    // ? helper layer hook
    led_strip_func_init(strip);

    ESP_LOGI(TAG, "LED strip initialized");
    return ESP_OK;
}

// ==================================================
// FREE
// ==================================================
esp_err_t led_strip_free(led_strip_t *strip)
{
    CHECK_ARG(strip);

    led_strip_func_deinit(strip);

    if (strip->channel) {
        rmt_disable(strip->channel);
        rmt_del_channel(strip->channel);
        strip->channel = NULL;
    }

    if (strip->encoder) {
        rmt_del_encoder(strip->encoder);
        strip->encoder = NULL;
    }

    free(strip->buf);
    strip->buf = NULL;

    return ESP_OK;
}

// ==================================================
// WRAPPERS ? HELPER LAYER
// ==================================================
esp_err_t led_strip_refresh(led_strip_t *strip)
{
    led_strip_func_refresh(strip);
    return ESP_OK;
}

esp_err_t led_strip_clear(led_strip_t *strip)
{
    led_strip_func_clear(strip);
    return ESP_OK;
}

esp_err_t led_strip_set_pixel(
    led_strip_t *strip,
    size_t index,
    rgb_t color
)
{
    led_strip_func_set_pixel(strip, index, color);
    return ESP_OK;
}

esp_err_t led_strip_fill(
    led_strip_t *strip,
    size_t start,
    size_t len,
    rgb_t color
)
{
    led_strip_func_fill(strip, start, len, color);
    return ESP_OK;
}