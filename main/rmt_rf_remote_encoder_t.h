#ifndef RMT_RF_REMOTE_ENCODER_T_H
#define RMT_RF_REMOTE_ENCODER_T_H

#include "driver/rmt_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    rmt_symbol_word_t reset_code;
    rmt_symbol_word_t preamble_time;
    rmt_symbol_word_t dead_time;
    int state;
} rmt_rf_remote_encoder_t;

#endif // RMT_RF_REMOTE_ENCODER_T_H
#ifdef __cplusplus
}
#endif