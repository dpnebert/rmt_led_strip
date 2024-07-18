/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_check.h"
#include "rf_remote_encoder.h"
#include "rmt_rf_remote_encoder_t.h"
#include "rmt_rf_remote_t.h"

static const char *TAG = "rf_remote_encoder";



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