#ifndef RF_REMOTE_T_H
#define RF_REMOTE_T_H

#include "rf_remote_payload_t.h"

typedef struct {
    rf_remote_payload_t payload = {
        .full = 0
    };
    uint32_t _address = 0;
} rf_remote_t;

#endif // RF_REMOTE_T_H