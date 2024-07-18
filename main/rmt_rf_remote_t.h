#ifndef RMT_RF_CHANNEL_H
#define RMT_RF_CHANNEL_H

#include "driver/rmt_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    rmt_channel_handle_t channel_handle = NULL;
    rmt_encoder_handle_t encoder_handle = NULL;
    rmt_transmit_config_t tx_config;
} rmt_rf_channel_t;


#endif // RMT_RF_CHANNEL_H
#ifdef __cplusplus
}
#endif