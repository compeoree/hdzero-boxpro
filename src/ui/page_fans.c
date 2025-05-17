#include "page_fans.h"

#include <stdio.h>

#include <log/log.h>
#include <minIni.h>

#include "core/app_state.h"
#include "core/common.hh"
#include "core/settings.h"
#include "driver/fans.h"
#include "driver/nct75.h"
#include "lang/language.h"
#include "ui/page_common.h"
#include "ui/page_fans.h"
#include "ui/ui_attribute.h"
#include "ui/ui_style.h"

typedef enum {
    FANS_MODE_NO_FAN = 0,
    FANS_MODE_TOP,
} fans_mode_t;

static lv_coord_t col_dsc[] = {106, 106, 106, 106, 92, 106, LV_GRID_TEMPLATE_LAST};
static lv_coord_t row_dsc[] = {40, 40, 40, 40, 40, 40, 40, 40, 40, 40, LV_GRID_TEMPLATE_LAST};

static fans_mode_t fans_mode = FANS_MODE_NO_FAN;

static btn_group_t btn_group_fans;

static slider_group_t slider_group;

static void update_visibility() {
    slider_enable(&slider_group, btn_group_fans.current != 0);

    if (btn_group_fans.current == 0) {
        lv_obj_clear_flag(pp_fans.p_arr.panel[1], FLAG_SELECTABLE);
    } else {
        lv_obj_add_flag(pp_fans.p_arr.panel[1], FLAG_SELECTABLE);
    }
}

static lv_obj_t *page_fans_create(lv_obj_t *parent, panel_arr_t *arr) {
    char buf[128];
    lv_obj_t *page = lv_menu_page_create(parent, NULL);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(page, 780, 600);
    lv_obj_add_style(page, &style_subpage, LV_PART_MAIN);
    lv_obj_set_style_pad_top(page, 62, 0);

    lv_obj_t *section = lv_menu_section_create(page);
    lv_obj_add_style(section, &style_submenu, LV_PART_MAIN);
    lv_obj_set_size(section, 780, 600);

    snprintf(buf, sizeof(buf), "%s:", _lang("Fans"));
    create_text(NULL, section, false, buf, LV_MENU_ITEM_BUILDER_VARIANT_2);

    lv_obj_t *cont = lv_obj_create(section);
    lv_obj_set_size(cont, 780, 600);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_style(cont, &style_context, LV_PART_MAIN);

    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);

    create_select_item(arr, cont);

    create_btn_group_item(&btn_group_fans, cont, 2, _lang("Auto Control"), _lang("On"), _lang("Off"), "", "", 0);
    create_slider_item(&slider_group, cont, _lang("Fan Speed"), MAX_FAN_TOP, 2, 1);
    lv_slider_set_range(slider_group.slider, MIN_FAN_TOP, MAX_FAN_TOP);

    snprintf(buf, sizeof(buf), "< %s", _lang("Back"));
    create_label_item(cont, buf, 1, 2, 1);

    btn_group_set_sel(&btn_group_fans, !g_setting.fans.auto_mode);

    lv_slider_set_value(slider_group.slider, g_setting.fans.top_speed, LV_ANIM_OFF);

    snprintf(buf, sizeof(buf), "%d", g_setting.fans.top_speed);
    lv_label_set_text(slider_group.label, buf);

    update_visibility();

    return page;
}

static void fans_top_speed_inc() {
    char buf[12];
    int32_t value = lv_slider_get_value(slider_group.slider);

    if (value < MAX_FAN_TOP)
        value += 1;

    lv_slider_set_value(slider_group.slider, value, LV_ANIM_OFF);

    snprintf(buf, sizeof(buf), "%d", value);
    lv_label_set_text(slider_group.label, buf);

    fans_top_setspeed(value);

    g_setting.fans.top_speed = value;
    ini_putl("fans", "top_speed", value, SETTING_INI);
}

static void fans_top_speed_dec() {
    char buf[12];
    int32_t value = lv_slider_get_value(slider_group.slider);

    if (value > MIN_FAN_TOP)
        value -= 1;

    lv_slider_set_value(slider_group.slider, value, LV_ANIM_OFF);
    snprintf(buf, sizeof(buf), "%d", value);
    lv_label_set_text(slider_group.label, buf);

    fans_top_setspeed(value);

    g_setting.fans.top_speed = (uint8_t)value;
    ini_putl("fans", "top_speed", value, SETTING_INI);
}

static void fans_speed_inc(void) {
    if (fans_mode == FANS_MODE_TOP) {
        fans_top_speed_inc();
    }
}

static void fans_speed_dec(void) {
    if (fans_mode == FANS_MODE_TOP) {
        fans_top_speed_dec();
    }
}

static void page_fans_exit_slider() {
    lv_obj_t *slider;
    if (fans_mode == FANS_MODE_TOP) {
        slider = slider_group.slider;
    } else {
        return;
    }

    app_state_push(APP_STATE_SUBMENU);
    lv_obj_add_style(slider, &style_silder_main, LV_PART_MAIN);
    fans_mode = FANS_MODE_NO_FAN;
}

static void page_fans_mode_exit() {
    if (fans_mode != FANS_MODE_NO_FAN) {
        page_fans_exit_slider();
    }
}

static void page_fans_mode_on_roller(uint8_t key) {
    if (g_app_state != APP_STATE_SUBMENU_ITEM_FOCUSED) {
        return;
    }

    if (key == DIAL_KEY_UP) {
        fans_speed_dec();
    } else if (key == DIAL_KEY_DOWN) {
        fans_speed_inc();
    }
}

static void page_fans_mode_on_click(uint8_t key, int sel) {
    lv_obj_t *slider = slider_group.slider;

    if (sel == 0) {
        btn_group_toggle_sel(&btn_group_fans);
        g_setting.fans.auto_mode = btn_group_get_sel(&btn_group_fans) == 0;
        settings_put_bool("fans", "auto", g_setting.fans.auto_mode);
        update_visibility();
        return;
    } else if (sel == 1) {
        slider = slider_group.slider;
        fans_mode = FANS_MODE_TOP;
    } else {
        return;
    }

    if (g_app_state == APP_STATE_SUBMENU_ITEM_FOCUSED) {
        page_fans_exit_slider();
    } else {
        app_state_push(APP_STATE_SUBMENU_ITEM_FOCUSED);
        lv_obj_add_style(slider, &style_silder_select, LV_PART_MAIN);
    }
}

void step_topfan() {
    char str[10];

    if (g_setting.fans.top_speed == MAX_FAN_TOP)
        g_setting.fans.top_speed = MIN_FAN_TOP;
    else
        g_setting.fans.top_speed++;

    fans_top_setspeed(g_setting.fans.top_speed);
    ini_putl("fans", "top_speed", g_setting.fans.top_speed, SETTING_INI);

    lv_slider_set_value(slider_group.slider, g_setting.fans.top_speed, LV_ANIM_OFF);
    snprintf(str, sizeof(str), "%d", g_setting.fans.top_speed);
    lv_label_set_text(slider_group.label, str);
}

///////////////////////////////////////////////////////////////////////////////
// Auto control top fans
static uint16_t respeed_cnt = 0;
static bool respeeding = false;

uint8_t adj_speed(uint8_t cur_speed, int tempe) {
    uint8_t new_speed = cur_speed;

    if (tempe > FAN_TEMPERATURE_THR_H) {
        if (new_speed != MAX_FAN_TOP) {
            new_speed++;
            respeeding = true;
            respeed_cnt = 0;
        }
    } else if (tempe < FAN_TEMPERATURE_THR_L) {
        if (new_speed != MIN_FAN_TOP) {
            new_speed--;
            respeeding = true;
            respeed_cnt = 0;
        }
    }

    if (cur_speed != new_speed)
        LOGI("%s Fan speed: %d (T=%d)", "Top", new_speed, tempe);

    return new_speed;
}

void fans_auto_ctrl_core(int tempe, bool binit) {
    static uint8_t speed = 0;
    uint8_t new_spd;

    //////////////////////////////////////////////////////////////////////////////////
    // reinit auto speed
    if (binit) {
        speed = 2; // Initial fan speed for auto mode
        respeed_cnt = 0;
        respeeding = false;
        fans_top_setspeed(speed);
    }

    if (respeeding) {
        respeed_cnt++;
        if (respeed_cnt == RESPEED_WAIT_TIME) {
            respeeding = false;
            respeed_cnt = 0;
        }
        return;
    }

    new_spd = adj_speed(speed, tempe);
    if (new_spd != speed) {
        speed = new_spd;
        fans_top_setspeed(speed);
    }
}

bool rescue_from_hot() {
    static fan_speed_t speed_saved;
    static bool respeeding = false;

    // Top
    if (g_temperature.top > TOP_TEMPERATURE_RISKH) {
        if (!respeeding) {
            speed_saved.top = fan_speed.top;
            respeeding = true;
            fans_top_setspeed(MAX_FAN_TOP);
            LOGI("Top fan: rescue ON.\n");
        }
    } else if (respeeding && (g_temperature.top < TOP_TEMPERATURE_NORM)) {
        fans_top_setspeed(speed_saved.top);
        respeeding = false;
        LOGI("Top fan: rescue OFF.");
    }

    g_temperature.is_rescuing = respeeding;
    g_temperature.is_overheat = (g_temperature.top > TOP_TEMPERATURE_RISKH);

    return g_temperature.is_rescuing;
}

void fans_auto_ctrl() {
    static uint8_t auto_mode_d;
    static fan_speed_t speed;
    uint8_t binit_r, binit_f;

    if (rescue_from_hot())
        return;

    binit_r = (auto_mode_d == 0) && (g_setting.fans.auto_mode == 1); // Manual mode -> Auto
    binit_f = (auto_mode_d == 1) && (g_setting.fans.auto_mode == 0); // Auto   mode -> manual
    auto_mode_d = g_setting.fans.auto_mode;

    if (g_setting.fans.auto_mode) {
        fans_auto_ctrl_core(g_temperature.top, binit_r);
    } else {
        if (binit_f)
            speed.top = 0xFF;

        if (speed.top != g_setting.fans.top_speed) {
            fans_top_setspeed(g_setting.fans.top_speed);
            speed.top = g_setting.fans.top_speed;
        }
    }
}

void change_topfan(uint8_t key) {
    if (key == DIAL_KEY_UP) {
        fans_top_speed_inc();
    } else if (key == DIAL_KEY_DOWN) {
        fans_top_speed_dec();
    }
}

page_pack_t pp_fans = {
    .p_arr = {
        .cur = 0,
        .max = 3,
    },
    .name = "Fan",
    .create = page_fans_create,
    .enter = NULL,
    .exit = page_fans_mode_exit,
    .on_created = NULL,
    .on_update = NULL,
    .on_roller = page_fans_mode_on_roller,
    .on_click = page_fans_mode_on_click,
    .on_right_button = NULL,
};