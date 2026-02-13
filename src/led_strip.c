#include "led_strip.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_rom_sys.h"

#define TAG "led_strip"

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
// Encode one pixel (brightness + order)
// ==================================================
static inline void encode_pixel(
    uint8_t *dst,
    rgb_t c,
    uint8_t brightness,
    led_strip_order_t order
)
{
    uint8_t r = (uint8_t)((c.r * brightness) >> 8);
    uint8_t g = (uint8_t)((c.g * brightness) >> 8);
    uint8_t b = (uint8_t)((c.b * brightness) >> 8);

    switch (order) {
        case LED_ORDER_RGB:
            dst[0] = r; dst[1] = g; dst[2] = b;
            break;
        case LED_ORDER_BRG:
            dst[0] = b; dst[1] = r; dst[2] = g;
            break;
        case LED_ORDER_GRB:
        default:
            dst[0] = g; dst[1] = r; dst[2] = b;
            break;
    }
}

// ==================================================
// INIT
// ==================================================
esp_err_t led_strip_init(led_strip_t *strip)
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

    // -----------------------------
    // Defaults
    // -----------------------------
    if (strip->brightness == 0)
        strip->brightness = 255;

    if (strip->order > LED_ORDER_BRG)
        strip->order = LED_ORDER_GRB;

    ESP_LOGI(TAG, "LED strip initialized (RMT TX, ESP-IDF 5.x)");
    return ESP_OK;
}

// ==================================================
// FREE
// ==================================================
esp_err_t led_strip_free(led_strip_t *strip)
{
    CHECK_ARG(strip);

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
// REFRESH (send buffer to LEDs)
// ==================================================
esp_err_t led_strip_refresh(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->buf);

    rmt_transmit_config_t cfg = {
        .loop_count = 0
    };

    CHECK(rmt_transmit(
        strip->channel,
        strip->encoder,
        strip->buf,
        strip->length * 3,
        &cfg
    ));

    CHECK(rmt_tx_wait_all_done(strip->channel, portMAX_DELAY));
    esp_rom_delay_us(RESET_US);

    return ESP_OK;
}

// ==================================================
// CLEAR
// ==================================================
esp_err_t led_strip_clear(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->buf);
    memset(strip->buf, 0, strip->length * 3);
    return led_strip_refresh(strip);
}

// ==================================================
// PIXELS
// ==================================================
esp_err_t led_strip_set_pixel(led_strip_t *strip, size_t index, rgb_t color)
{
    CHECK_ARG(strip && strip->buf && index < strip->length);

    encode_pixel(
        &strip->buf[index * 3],
        color,
        strip->brightness,
        strip->order
    );

    return ESP_OK;
}

esp_err_t led_strip_fill(led_strip_t *strip, size_t start, size_t len, rgb_t color)
{
    CHECK_ARG(strip && strip->buf && start + len <= strip->length);

    for (size_t i = start; i < start + len; i++)
        led_strip_set_pixel(strip, i, color);

    return ESP_OK;
}