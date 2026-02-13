#include "led_strip.h"

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_rom_sys.h"

#define TAG "led_strip_core"

#define T0H_NS   400
#define T0L_NS   850
#define T1H_NS   800
#define T1L_NS   450
#define RESET_US 60
#define NS_TO_TICKS(ns) ((ns) / 100)

#define CHECK(x)     do { esp_err_t r = (x); if (r != ESP_OK) return r; } while (0)
#define CHECK_ARG(x) do { if (!(x)) return ESP_ERR_INVALID_ARG; } while (0)

static inline void encode_pixel(uint8_t *dst, rgb_t c)
{
    dst[0] = c.g;
    dst[1] = c.r;
    dst[2] = c.b;
}

esp_err_t led_strip_core_init(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->length > 0);

    strip->buf = calloc(strip->length * 3, 1);
    if (!strip->buf) return ESP_ERR_NO_MEM;

    rmt_tx_channel_config_t tx_cfg = {
        .gpio_num = strip->gpio,
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };

    CHECK(rmt_new_tx_channel(&tx_cfg, &strip->channel));

    rmt_bytes_encoder_config_t enc_cfg = {
        .bit0 = {1, NS_TO_TICKS(T0H_NS), 0, NS_TO_TICKS(T0L_NS)},
        .bit1 = {1, NS_TO_TICKS(T1H_NS), 0, NS_TO_TICKS(T1L_NS)},
        .flags.msb_first = true,
    };

    CHECK(rmt_new_bytes_encoder(&enc_cfg, &strip->encoder));
    CHECK(rmt_enable(strip->channel));

    ESP_LOGI(TAG, "Core initialized");
    return ESP_OK;
}

esp_err_t led_strip_core_free(led_strip_t *strip)
{
    CHECK_ARG(strip);

    rmt_disable(strip->channel);
    rmt_del_channel(strip->channel);
    rmt_del_encoder(strip->encoder);

    free(strip->buf);
    strip->buf = NULL;

    return ESP_OK;
}

esp_err_t led_strip_core_refresh(led_strip_t *strip)
{
    CHECK_ARG(strip && strip->buf);

    rmt_transmit_config_t cfg = { .loop_count = 0 };
    CHECK(rmt_transmit(strip->channel, strip->encoder,
                       strip->buf, strip->length * 3, &cfg));

    CHECK(rmt_tx_wait_all_done(strip->channel, portMAX_DELAY));
    esp_rom_delay_us(RESET_US);
    return ESP_OK;
}

esp_err_t led_strip_core_set_pixel(led_strip_t *strip, size_t index, rgb_t c)
{
    CHECK_ARG(strip && index < strip->length);
    encode_pixel(&strip->buf[index * 3], c);
    return ESP_OK;
}