#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void TP2825_close();
void TP2825_open();

void TP2825_init(int ch_sel, int is_pal); // ch_sel: 0=rtc6715; 1=AV_in
void TP2825_Switch_Mode(int is_pal);
void TP2825_Switch_CH(uint8_t sel); // ch_sel: 0=rtc6715; 1=AV_in

#ifdef __cplusplus
}
#endif
