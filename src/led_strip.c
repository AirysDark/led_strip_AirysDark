#include "led_strip.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "led_strip_core"

// ==================================================
// WS2812 timing (nanoseconds)
// RMT resolution = 10 MHz ? 1 tick = 100 ns
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
// Encode pixel (GRB only ? helper handles order)
// ==================================================
static inline void encode_pixel(uint8_t *dst, rgb_t c)
{
    dst[0] = c.g;
    dst[1] = c.r;
    dst[2] = c.b;
}

// ==================================================
// Reset symbol (static, persistent)
// ==================================================
static const rmt_symbol_word_t reset_symbol = {
    .level0 = 0,
    .duration0 = NS_TO_TICKS(RESET_US * 1000),
    .level1 = 0,
    .duration1 = 0,
};

// ==================================================
// CORE INIT
// ==================================================
esp_err_t led_strip_core_init(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->length > 0);

    strip->buf = calloc(strip->length * 3, 1);
    if (!strip->buf)
        return ESP_ERR_NO_MEM;

    // -----------------------------
    // RMT TX channel
    // -----------------------------
    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = strip->gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };

    CHECK(rmt_new_tx_channel(&tx_cfg, &strip->channel));

    // -----------------------------
    // WS2812 byte encoder
    // -----------------------------
    rmt_bytes_encoder_config_t bytes_cfg = {
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
        .flags = {
            .msb_first = 1,
        },
    };

    CHECK(rmt_new_bytes_encoder(&bytes_cfg, &strip->bytes_encoder));

    // -----------------------------
    // Reset encoder (WS2812 latch)
    // -----------------------------
    rmt_copy_encoder_config_t copy_cfg = {};
    CHECK(rmt_new_copy_encoder(&copy_cfg, &strip->reset_encoder));

    // -----------------------------
    // Composite encoder (data + reset)
    // -----------------------------
    rmt_encoder_handle_t encoders[] = {
        strip->bytes_encoder,
        strip->reset_encoder
    };

    rmt_composite_encoder_config_t comp_cfg = {
        .encoders = encoders,
        .num_encoders = 2,
    };

    CHECK(rmt_new_composite_encoder(&comp_cfg, &strip->composite_encoder));
    CHECK(rmt_enable(strip->channel));

    ESP_LOGI(TAG, "LED strip core initialized (composite encoder)");
    return ESP_OK;
}

// ==================================================
// CORE FREE
// ==================================================
esp_err_t led_strip_core_free(led_strip_t *strip)
{
    CHECK_ARG(strip);

    if (strip->channel) {
        rmt_disable(strip->channel);
        rmt_del_channel(strip->channel);
        strip->channel = NULL;
    }

    if (strip->composite_encoder) {
        rmt_del_encoder(strip->composite_encoder);
        strip->composite_encoder = NULL;
    }

    if (strip->bytes_encoder) {
        rmt_del_encoder(strip->bytes_encoder);
        strip->bytes_encoder = NULL;
    }

    if (strip->reset_encoder) {
        rmt_del_encoder(strip->reset_encoder);
        strip->reset_encoder = NULL;
    }

    free(strip->buf);
    strip->buf = NULL;

    return ESP_OK;
}

// ==================================================
// CORE REFRESH (DATA + RESET)
// ==================================================
esp_err_t led_strip_core_refresh(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->buf);

    rmt_transmit_config_t cfg = {
        .loop_count = 0
    };

    CHECK(rmt_transmit(
        strip->channel,
        strip->composite_encoder,
        strip->buf,
        strip->length * 3,
        &cfg
    ));

    CHECK(rmt_tx_wait_all_done(strip->channel, portMAX_DELAY));
    return ESP_OK;
}

// ==================================================
// CORE SET PIXEL
// ==================================================
esp_err_t led_strip_core_set_pixel(led_strip_t *strip, size_t index, rgb_t c)
{
    CHECK_ARG(strip && strip->buf && index < strip->length);
    encode_pixel(&strip->buf[index * 3], c);
    return ESP_OK;
}