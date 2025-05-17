#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include <lvgl/lvgl.h>

#include "player/media.h"

#define MPLAYER_SCR_WIDTH  1280
#define MPLAYER_SCR_HEIGHT 720
#define MPLAYER_CB_WIDTH   460
#define MPLAYER_CB_HEIGHT  56
#define MPLAYER_BTN_WIDTH  32
#define MPLAYER_BTN_HEIGHT 32
#define MPLAYER_BTN_GAP    4
#define MPLAYER_SLD_WIDTH  260
#define MPLAYER_SLD_HEIGHT 2
#define LV_COLOR_MAKE_32(a, r, g, b) \
    _LV_COLOR_MAKE_TYPE_HELPER {     \
        { b, g, r, a }               \
    }

#define MAX_STARS 10

typedef struct {
    lv_obj_t *_btn;
    lv_obj_t *_slider;
    lv_obj_t *_label; // text format as "mm:ss(playing)/mm:ss(total)"
    lv_obj_t *_stars[MAX_STARS];
    bool is_playing;
    int32_t value;
    int32_t range;

    bool enable;
    lv_obj_t *bg;
    lv_obj_t *bar;
} player_control_t;

uint8_t mplayer_on_key(uint8_t key);
void mplayer_set_time(uint32_t now, uint32_t duration);
void mplayer_file(char *fname);

#ifdef __cplusplus
}
#endif
