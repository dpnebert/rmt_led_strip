#ifndef RF_REMOTE_PAYLOAD_T_H
#define RF_REMOTE_PAYLOAD_T_H
#include "driver/rmt_tx.h"
typedef union {
    struct {
        uint8_t unused_bit0: 1;
        uint8_t unused_bit1: 1;
        uint8_t unused_bit2: 1;
        uint8_t unused_bit3: 1;
        uint8_t unused_bit4: 1;
        uint8_t unused_bit5: 1;
        uint8_t unused_bit6: 1;
        uint8_t unused_bit7: 1;
        uint8_t unused_bit8: 1;
        uint8_t unused_bit9: 1;
        uint8_t unused_bit10: 1;
        uint8_t unused_bit11: 1;
        uint8_t unused_bit12: 1;
        uint8_t unused_bit13: 1;
        uint8_t unused_bit14: 1;
        uint8_t unused_bit15: 1;
        uint8_t unused_bit16: 1;
        uint8_t unused_bit17: 1;
        uint8_t preset: 1;
        uint8_t sync: 1;
        uint8_t stop_sync: 1;
        uint8_t down: 1;
        uint8_t stop: 1;
        uint8_t up: 1;
        uint8_t unused_bit24: 1;
        uint8_t unused_bit25: 1;
        uint8_t unused_bit26: 1;
        uint8_t channel_1: 1;
        uint8_t channel_2: 1;
        uint8_t channel_3: 1;
        uint8_t channel_4: 1;
        uint8_t channel_5: 1;
        uint32_t address: 31;
        uint8_t pressedReleased: 1;
    } field;
    uint64_t full;
} rf_remote_payload_t;
#endif // RF_REMOTE_PAYLOAD_T_H