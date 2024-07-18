
#pragma once

#include <stdint.h>
#include "driver/rmt_encoder.h"
#include "rf_remote_encoder_config_t.h"
#include "rf_remote_payload_t.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
 * @brief Create RMT encoder for encoding LED strip pixels into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating led strip encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_rf_remote_encoder(rf_remote_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

#ifdef __cplusplus
}
#endif
