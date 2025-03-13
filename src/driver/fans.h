#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    uint8_t top;
} fan_speed_t;

void fans_top_setspeed(uint8_t speed);

extern fan_speed_t fan_speed;

#ifdef __cplusplus
}
#endif
