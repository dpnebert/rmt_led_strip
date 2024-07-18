#ifndef RMT_RF_REMOTE_ENCODER_CONFIG_T_H
#define RMT_RF_REMOTE_ENCODER_CONFIG_T_H

#include "driver/rmt_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Type of led strip encoder configuration
 */
typedef struct {
    uint32_t resolution; /*!< Encoder resolution, in Hz */
} rf_remote_encoder_config_t;

#endif // RMT_RF_REMOTE_ENCODER_CONFIG_T_H
#ifdef __cplusplus
}
#endif