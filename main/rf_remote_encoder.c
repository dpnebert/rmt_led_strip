/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_check.h"
#include "rf_remote_encoder.h"
#include "rmt_rf_remote_encoder_t.h"
#include "rmt_rf_remote_t.h"

static const char *TAG = "rmt_encoder";



// static size_t rmt_encode_rf_remote                 (rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
static size_t encode_cb(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    rmt_rf_remote_encoder_t *_encoder = __containerof(encoder, rmt_rf_remote_encoder_t, base);

    rf_remote_payload_t data = *(rf_remote_payload_t *)primary_data;

    rmt_encoder_handle_t bytes_encoder = _encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = _encoder->copy_encoder;


    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    int state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    
    ESP_LOGI(TAG, "data.full: %llu", data.full);

    uint64_t reverse_data = 0;
    for(int i = 0; i < 64; i++)
    {
        reverse_data |= ((data.full >> i) & 1) << (63 - i);
    }   

    switch (state) {

    case 0: // encode and add pressedRelased
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &_encoder->preamble_time, sizeof(_encoder->preamble_time), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            state++; // we can only switch to next state when current encoder finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            *ret_state = (rmt_encode_state_t)state;
            return encoded_symbols;
        }
    case 1: // encode and add pressedRelased
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, &reverse_data, sizeof(reverse_data), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            state++;//state = RMT_ENCODING_COMPLETE; // we can only switch to next state when current encoder finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            *ret_state = (rmt_encode_state_t)state;
            return encoded_symbols;
        }
    case 2:
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &_encoder->dead_time, sizeof(_encoder->dead_time), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            state++; // we can only switch to next state when current encoder finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            *ret_state = (rmt_encode_state_t)state;
            return encoded_symbols;
        }
    case 3:
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &_encoder->dead_time, sizeof(_encoder->dead_time), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            state = RMT_ENCODING_COMPLETE; // we can only switch to next state when current encoder finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            *ret_state = (rmt_encode_state_t)state;
            return encoded_symbols;
        }
    }
    *ret_state = (rmt_encode_state_t)state;
    return encoded_symbols;
}

static esp_err_t delete_cb(rmt_encoder_t *encoder)
{
    rmt_rf_remote_encoder_t *_encoder = __containerof(encoder, rmt_rf_remote_encoder_t, base);
    rmt_del_encoder(_encoder->bytes_encoder);
    rmt_del_encoder(_encoder->copy_encoder);
    free(encoder);
    return ESP_OK;
}

static esp_err_t reset_cb(rmt_encoder_t *encoder)
{
    rmt_rf_remote_encoder_t *_encoder = __containerof(encoder, rmt_rf_remote_encoder_t, base);
    rmt_encoder_reset(_encoder->bytes_encoder);
    rmt_del_encoder(_encoder->copy_encoder);
    return ESP_OK;
}



esp_err_t rmt_new_rf_remote_encoder(rf_remote_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder) {
    esp_err_t ret = ESP_OK;
    rmt_rf_remote_encoder_t *remote_encoders = NULL;
    remote_encoders = (rmt_rf_remote_encoder_t*)rmt_alloc_encoder_mem(sizeof(rmt_rf_remote_encoder_t));

    if(NULL == remote_encoders) {
        ESP_LOGE(TAG, "Failed to allocate memory for remote encoder");
        return ESP_ERR_NO_MEM;
    } else {
        ESP_LOGI(TAG, "Memory allocated for remote encoder");
    }

    remote_encoders->base.encode = encode_cb;
    remote_encoders->base.del = delete_cb;
    remote_encoders->base.reset = reset_cb;
    
    remote_encoders->preamble_time = (rmt_symbol_word_t){};
    //remote_encoders->preamble_time.duration0 = CONFIG_PREAMBLE_PULSE_WIDTH;
    remote_encoders->preamble_time.duration0 = 1000;
    remote_encoders->preamble_time.level0 = 1;
    //remote_encoders->preamble_time.duration1 = CONFIG_PREAMBLE_PULSE_WIDTH;
    remote_encoders->preamble_time.duration1 = 1000;
    remote_encoders->preamble_time.level1 = 1;

    remote_encoders->dead_time = (rmt_symbol_word_t){};
    //remote_encoders->dead_time.duration0 = CONFIG_DEADTIME_HALF_PULSE_WIDTH;
    remote_encoders->dead_time.duration0 = 3000;
    remote_encoders->dead_time.level0 = 0;
    //remote_encoders->dead_time.duration1 = CONFIG_DEADTIME_HALF_PULSE_WIDTH;
    remote_encoders->dead_time.duration1 = 3000;
    remote_encoders->dead_time.level1 = 0;

    rmt_symbol_word_t data_bit_0 = (rmt_symbol_word_t){};
    data_bit_0.level0 = 0;
    //data_bit_0.duration0 = CONFIG_DATA_HIGH_PULSE_WIDTH;
    data_bit_0.duration0 = 2000;
    data_bit_0.level1 = 1;
    //data_bit_0.duration1 = CONFIG_DATA_LOW_PULSE_WIDTH;
    data_bit_0.duration1 = 1000;

    rmt_symbol_word_t data_bit_1 = (rmt_symbol_word_t){};
    data_bit_1.level0 = 0;
    data_bit_1.duration0 = 1000;
    data_bit_1.level1 = 1;
    data_bit_1.duration1 = 2000;

    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = data_bit_0,
        .bit1 = data_bit_1,
        .flags = {
            .msb_first = 0
        }
    };

    
    ret = rmt_new_bytes_encoder(&bytes_encoder_config, &remote_encoders->bytes_encoder);
    if(ESP_OK == ret) {
        ESP_LOGI(TAG, "Bytes encoder created");
    } else {
        ESP_LOGE(TAG, "Failed to create bytes encoder");
    }

    rmt_copy_encoder_config_t copy_encoder_config = {};


    ret = rmt_new_copy_encoder(&copy_encoder_config, &remote_encoders->copy_encoder);
    if(ESP_OK == ret) {
        ESP_LOGI(TAG, "Copy encoder created");
    } else {
        ESP_LOGE(TAG, "Failed to create copy encoder");
    }

    *ret_encoder = &remote_encoders->base;

    return ESP_OK;
}















// static size_t rmt_encode_rf_remote(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
// {
//     rmt_rf_remote_encoder_t *led_encoder = __containerof(encoder, rmt_rf_remote_encoder_t, base);
//     rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
//     rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
//     rmt_encode_state_t session_state = RMT_ENCODING_RESET;
//     rmt_encode_state_t state = RMT_ENCODING_RESET;
//     size_t encoded_symbols = 0;
//     switch (led_encoder->state) {
//     case 0: // send RGB data
//         encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
//         if (session_state & RMT_ENCODING_COMPLETE) {
//             led_encoder->state = 1; // switch to next state when current encoding session finished
//         }
//         if (session_state & RMT_ENCODING_MEM_FULL) {
//             state |= RMT_ENCODING_MEM_FULL;
//             goto out; // yield if there's no free space for encoding artifacts
//         }
//     // fall-through
//     case 1: // send reset code
//         encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
//                                                 sizeof(led_encoder->reset_code), &session_state);
//         if (session_state & RMT_ENCODING_COMPLETE) {
//             led_encoder->state = RMT_ENCODING_RESET; // back to the initial encoding session
//             state |= RMT_ENCODING_COMPLETE;
//         }
//         if (session_state & RMT_ENCODING_MEM_FULL) {
//             state |= RMT_ENCODING_MEM_FULL;
//             goto out; // yield if there's no free space for encoding artifacts
//         }
//     }
// out:
//     *ret_state = state;
//     return encoded_symbols;
// }

// static esp_err_t rmt_del_rf_remote_encoder(rmt_encoder_t *encoder)
// {
//     rmt_rf_remote_encoder_t *led_encoder = __containerof(encoder, rmt_rf_remote_encoder_t, base);
//     rmt_del_encoder(led_encoder->bytes_encoder);
//     rmt_del_encoder(led_encoder->copy_encoder);
//     free(led_encoder);
//     return ESP_OK;
// }

// static esp_err_t rmt_rf_remote_encoder_reset(rmt_encoder_t *encoder)
// {
//     rmt_rf_remote_encoder_t *led_encoder = __containerof(encoder, rmt_rf_remote_encoder_t, base);
//     rmt_encoder_reset(led_encoder->bytes_encoder);
//     rmt_encoder_reset(led_encoder->copy_encoder);
//     led_encoder->state = RMT_ENCODING_RESET;
//     return ESP_OK;
// }

// esp_err_t rmt_new_rf_remote_encoder(const rf_remote_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder)
// {
//     esp_err_t ret = ESP_OK;
//     rmt_rf_remote_encoder_t *led_encoder = NULL;
//     ESP_GOTO_ON_FALSE(config && ret_encoder, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
//     led_encoder = rmt_alloc_encoder_mem(sizeof(rmt_rf_remote_encoder_t));
//     ESP_GOTO_ON_FALSE(led_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for led strip encoder");
//     led_encoder->base.encode = rmt_encode_rf_remote;
//     led_encoder->base.del = rmt_del_rf_remote_encoder;
//     led_encoder->base.reset = rmt_rf_remote_encoder_reset;
//     // different led strip might have its own timing requirements, following parameter is for WS2812
//     rmt_bytes_encoder_config_t bytes_encoder_config = {
//         .bit0 = {
//             .level0 = 1,
//             .duration0 = 0.3 * config->resolution / 1000000, // T0H=0.3us
//             .level1 = 0,
//             .duration1 = 0.9 * config->resolution / 1000000, // T0L=0.9us
//         },
//         .bit1 = {
//             .level0 = 1,
//             .duration0 = 0.9 * config->resolution / 1000000, // T1H=0.9us
//             .level1 = 0,
//             .duration1 = 0.3 * config->resolution / 1000000, // T1L=0.3us
//         },
//         .flags.msb_first = 0,
//     };
//     ESP_GOTO_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder), err, TAG, "create bytes encoder failed");
//     rmt_copy_encoder_config_t copy_encoder_config = {};
//     ESP_GOTO_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder), err, TAG, "create copy encoder failed");

//     uint32_t reset_ticks = config->resolution / 1000000 * 50 / 2; // reset code duration defaults to 50us
//     led_encoder->reset_code = (rmt_symbol_word_t) {
//         .level0 = 0,
//         .duration0 = reset_ticks,
//         .level1 = 0,
//         .duration1 = reset_ticks,
//     };
//     *ret_encoder = &led_encoder->base;
//     return ESP_OK;
// err:
//     if (led_encoder) {
//         if (led_encoder->bytes_encoder) {
//             rmt_del_encoder(led_encoder->bytes_encoder);
//         }
//         if (led_encoder->copy_encoder) {
//             rmt_del_encoder(led_encoder->copy_encoder);
//         }
//         free(led_encoder);
//     }
//     return ret;
// }
