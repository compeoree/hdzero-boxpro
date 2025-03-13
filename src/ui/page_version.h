#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

#include "ui/ui_main_menu.h"

#define CURRENT_VER_MAX (64)
#define COMMIT_VER_MAX  (10)

enum {
    ROW_PRO_CUR_VERSION = 0,
    ROW_PRO_RESET_ALL_SETTINGS,
    ROW_PRO_UPDATE_GOGGLE,
    ROW_PRO_UPDATE_ESP32,
    ROW_PRO_BACK,

    ROW_PRO_COUNT
} ROW_PRO;

enum {
    ROW_LITE_CUR_VERSION = 0,
    ROW_LITE_RESET_ALL_SETTINGS,
    ROW_LITE_UPDATE_GOGGLE,
    ROW_LITE_BACK,

    ROW_LITE_COUNT
} ROW_LITE;
#define ROW_PAGE_VERSION_COUNT (g_setting.is_pro ? ROW_PRO_COUNT : ROW_LITE_COUNT)

typedef struct {
    uint8_t rx;
    uint8_t va;
    uint8_t app_major;
    uint8_t app_minor;
    uint8_t app_patch;
    char commit[COMMIT_VER_MAX];
    char current[CURRENT_VER_MAX];
} sys_version_t;

extern page_pack_t pp_version;

void version_update(int sel);
void process_bar_update(const int value0, const int value1);
void bar_update(int sel, int value);
void version_update_title();

int generate_current_version(sys_version_t *sys_ver);

void *thread_version(void *ptr);
uint8_t command_monitor(char *cmd);

#ifdef __cplusplus
}
#endif
