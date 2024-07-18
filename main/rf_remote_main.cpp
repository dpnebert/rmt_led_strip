/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "rf_remote_encoder.h"

#include "rmt_rf_remote_t.h"
#include "rf_remote_t.h"

#define RMT_RF_REMOTE_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_RF_REMOTE_GPIO_NUM      39

#define EXAMPLE_LED_NUMBERS         24
#define EXAMPLE_CHASE_SPEED_MS      10

extern "C" void app_main();

static const char *TAG = "example";


rmt_rf_remote_t* rf_rmt = new rmt_rf_remote_t();
//rf_remote_t* rf_remote = new rf_remote_t();

uint64_t payload[1];


void initRMT() {
    ESP_LOGI(TAG, "Create RMT TX channel config");
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = (gpio_num_t)RMT_RF_REMOTE_GPIO_NUM,
        .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
        .resolution_hz = RMT_RF_REMOTE_RESOLUTION_HZ,
        .mem_block_symbols = 128, // increase the block size can make the LED less flickering
        
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
        .intr_priority = 0,
        .flags = {
            .invert_out = 0,
            .with_dma = 0,
            .io_loop_back = 0,
            .io_od_mode = 0,
        }
    };
    ESP_LOGI(TAG, "Create RMT TX channel");
    esp_err_t ret = rmt_new_tx_channel(&tx_chan_config, &rf_rmt->channel_handle);
    if(ESP_OK != ret) {
        ESP_LOGE(TAG, "Failed to create RMT TX channel");
        return;
    } else {
        ESP_LOGI(TAG, "RMT TX channel created");
    }



    ESP_LOGI(TAG, "Install RF remote encoder");
    rf_remote_encoder_config_t encoder_config = {
        .resolution = RMT_RF_REMOTE_RESOLUTION_HZ,
    };
    ret = rmt_new_rf_remote_encoder(&encoder_config, &rf_rmt->encoder_handle);
    if(ESP_OK != ret) {
        ESP_LOGE(TAG, "Failed to install RF remote encoder");
        return;
    } else {
        ESP_LOGI(TAG, "RF remote encoder installed");
    }



    ESP_LOGI(TAG, "Enable RMT TX channel");
    ret = rmt_enable(rf_rmt->channel_handle);
    if(ESP_OK != ret) {
        ESP_LOGE(TAG, "Failed to enable RMT TX channel");
        return;
    } else {
        ESP_LOGI(TAG, "RMT TX channel enabled");
    }



    ESP_LOGI(TAG, "Configure RMT TX channel config");
    rf_rmt->tx_config = {
        .loop_count = -1, // no transfer loop
        .flags = {
            .eot_level = 0,
            .queue_nonblocking = 0
        }
    };
}

void app_main(void)
{
    payload[0] = 0x0;

    initRMT();

    while (1) {
        // Flush RGB values to LEDs
        esp_err_t ret = ESP_OK;
        ret = rmt_transmit(rf_rmt->channel_handle, rf_rmt->encoder_handle, payload, sizeof(payload), &rf_rmt->tx_config);
        if(ESP_OK != ret) {
            ESP_LOGE(TAG, "Failed to transmit RF remote data");
            return;
        } else {
            ESP_LOGI(TAG, "RF remote data transmitted");
        }


        
        
        payload[0] = payload[0] + 1;
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "Disable RMT TX channel (to stop continuous transmission)");
        ret = rmt_disable(rf_rmt->channel_handle);
        if(ESP_OK != ret) {
            ESP_LOGE(TAG, "Failed to disable RMT TX channel");
            return;
        } else {
            ESP_LOGI(TAG, "RMT TX channel disabled");
        }



        vTaskDelay(1000 / portTICK_PERIOD_MS);

        ESP_LOGI(TAG, "Enable RMT TX channel");
        ret = rmt_enable(rf_rmt->channel_handle);
        if(ESP_OK != ret) {
            ESP_LOGE(TAG, "Failed to enable RMT TX channel");
            return;
        } else {
            ESP_LOGI(TAG, "RMT TX channel enabled");
        }
        
    }
}
