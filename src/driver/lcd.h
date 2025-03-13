#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#define MIN_LCD_BRIGHTNESS 0
#define MAX_LCD_BRIGHTNESS 12

void LCD_display(int on);
void LCD_Brightness(uint8_t level);

#ifdef __cplusplus
}
#endif
