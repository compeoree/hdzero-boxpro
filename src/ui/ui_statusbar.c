#include "ui/ui_statusbar.h"

#include <stdio.h>

#include <lvgl/lvgl.h>

#include "core/battery.h"
#include "core/common.hh"
#include "core/osd.h"
#include "core/settings.h"
#include "driver/beep.h"
#include "ui/page_common.h"
#include "ui/page_playback.h"
#include "ui/page_storage.h"
#include "ui/page_wifi.h"
#include "ui/ui_porting.h"
#include "ui/ui_style.h"
#include "util/sdcard.h"

///////////////////////////////////////////////////////////////////////////////
// local
enum STATUS_PRO {
    STS_PRO_SDCARD,
    STS_PRO_SOURCE,
    STS_PRO_ELRS,
    STS_PRO_WIFI,
    STS_PRO_BATT,

    STS_PRO_TOTAL
};
enum STATUS_LITE {
    STS_LITE_SDCARD,
    STS_LITE_SOURCE,
    STS_LITE_BATT,

    STS_LITE_TOTAL
};

#define STS_SDCARD (g_setting.is_pro ? STS_PRO_SDCARD : STS_LITE_SDCARD)
#define STS_SOURCE (g_setting.is_pro ? STS_PRO_SOURCE : STS_LITE_SOURCE)
#define STS_ELRS   (STS_PRO_ELRS)
#define STS_WIFI   (STS_PRO_WIFI)
#define STS_BATT   (g_setting.is_pro ? STS_PRO_BATT : STS_LITE_BATT)
#define STS_TOTAL  (g_setting.is_pro ? STS_PRO_TOTAL : STS_LITE_TOTAL)

static lv_obj_t *label[STS_PRO_TOTAL];

static lv_obj_t *img_sdc;
LV_IMG_DECLARE(img_sdcard);
LV_IMG_DECLARE(img_noSdcard);

static lv_obj_t *img_battery;
LV_IMG_DECLARE(img_bat);
LV_IMG_DECLARE(img_lowBattery);

int statusbar_init(void) {
    char buf[128];

    static lv_coord_t col_dsc_pro[] = {176, 42, 178, 42, 178, 42, 178, 42, 178, 42, 178, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t col_dsc_lite[] = {176, 42, 392, 42, 392, 42, 392, LV_GRID_TEMPLATE_LAST};

    static lv_coord_t row_dsc[] = {32, 32, LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, DRAW_HOR_RES_FHD, 48);
    lv_obj_set_pos(cont, 0, 0);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_bg_color(cont, lv_color_make(19, 19, 19), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_style_pad_row(cont, 0, 0);
    lv_obj_set_style_pad_column(cont, 0, 0);

    lv_obj_set_style_grid_column_dsc_array(cont, (g_setting.is_pro ? col_dsc_pro : col_dsc_lite), 0);

    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);

    LV_IMG_DECLARE(img_logo);
    lv_obj_t *img0 = lv_img_create(cont);
    lv_img_set_src(img0, &img_logo);
    lv_obj_set_size(img0, 176, 48);
    lv_obj_set_grid_cell(img0, LV_GRID_ALIGN_CENTER, 0, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    img_sdc = lv_img_create(cont);
    lv_img_set_src(img_sdc, &img_sdcard);
    lv_obj_set_size(img_sdc, 42, 48);
    lv_obj_set_grid_cell(img_sdc, LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    LV_IMG_DECLARE(img_ic);
    lv_obj_t *img2 = lv_img_create(cont);
    lv_img_set_src(img2, &img_ic);
    lv_obj_set_size(img2, 42, 48);
    lv_obj_set_grid_cell(img2, LV_GRID_ALIGN_CENTER, 3, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
    if (g_setting.is_pro) {
        LV_IMG_DECLARE(img_esp);
        lv_obj_t *img3 = lv_img_create(cont);
        lv_img_set_src(img3, &img_esp);
        lv_obj_set_size(img3, 42, 48);
        lv_obj_set_grid_cell(img3, LV_GRID_ALIGN_CENTER, 5, 1,
                             LV_GRID_ALIGN_CENTER, 0, 1);

        LV_IMG_DECLARE(img_wifi);
        lv_obj_t *img4 = lv_img_create(cont);
        lv_img_set_src(img4, &img_wifi);
        lv_obj_set_size(img4, 42, 48);
        lv_obj_set_grid_cell(img4, LV_GRID_ALIGN_CENTER, 7, 1,
                             LV_GRID_ALIGN_CENTER, 0, 1);
    }

    img_battery = lv_img_create(cont);
    lv_img_set_src(img_battery, &img_bat);
    lv_obj_set_size(img_battery, 42, 48);
    lv_obj_set_grid_cell(img_battery, LV_GRID_ALIGN_CENTER, (g_setting.is_pro ? 9 : 5), 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    for (int i = 0; i < STS_TOTAL; ++i) {
        label[i] = lv_label_create(cont);
        lv_obj_set_width(label[i], (g_setting.is_pro ? 178 : 392)); /*Set smaller width to make the lines wrap*/
        lv_obj_set_style_text_align(label[i], LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_text_color(label[i], lv_color_hex(TEXT_COLOR_DEFAULT), 0);
        lv_obj_set_style_text_font(label[i], &lv_font_montserrat_18, 0);

        if (i == STS_SDCARD) {
            lv_label_set_long_mode(label[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
        } else {
            lv_label_set_long_mode(label[i], LV_LABEL_LONG_DOT); /*Break the long lines*/
        }

        lv_obj_set_grid_cell(label[i], LV_GRID_ALIGN_CENTER, ((i + 1) * 2), 1, LV_GRID_ALIGN_CENTER, 0, 1);
    }

    lv_label_set_text(label[STS_SDCARD], "SD Card                 ");
    lv_label_set_recolor(label[STS_SDCARD], true);

    if (g_source_info.source == SOURCE_HDZERO)
        sprintf(buf, "RF: HDZero %s", channel2str(1, g_setting.source.hdzero_band, g_setting.scan.channel & 0x7F));
    else if (g_source_info.source == SOURCE_HDMI_IN)
        sprintf(buf, "HDMI In");
    else if (g_source_info.source == SOURCE_AV_IN)
        sprintf(buf, "AV In");
    else if (g_source_info.source == SOURCE_AV_MODULE)
        sprintf(buf, "RF: Analog %s", channel2str(0, 0, g_setting.source.analog_channel));
    else
        sprintf(buf, " ");

    lv_label_set_text(label[STS_SOURCE], buf);

    if (g_setting.is_pro) {
        lv_label_set_text(label[STS_ELRS], "ELRS: Off");
        lv_label_set_text(label[STS_WIFI], "WiFi: Off");
    }

    lv_label_set_text(label[STS_BATT], "       ");
    return 0;
}

void statubar_update(void) {
    char buf[128];
    memset(buf, 0, sizeof(buf));

    // display battery voltage
    battery_get_voltage_str(buf);
    lv_label_set_text(label[STS_BATT], buf);

    {
#define BEEP_INTERVAL 20
        static uint8_t beep_gap = 0;

        const bool low = battery_is_low();
        if (low)
            lv_img_set_src(img_battery, &img_lowBattery);
        else
            lv_img_set_src(img_battery, &img_bat);

        switch (g_setting.power.warning_type) {
        case SETTING_POWER_WARNING_TYPE_BEEP:
            if (low) {
                if (beep_gap++ == BEEP_INTERVAL) {
                    beep();
                    beep_gap = 0;
                }
                lv_obj_set_style_text_color(label[STS_BATT], lv_color_hex(TEXT_COLOR_DEFAULT), 0);
            }
            break;

        case SETTING_POWER_WARNING_TYPE_VISUAL:
            if (low)
                lv_obj_set_style_text_color(label[STS_BATT], lv_color_make(255, 0, 0), 0);
            else
                lv_obj_set_style_text_color(label[STS_BATT], lv_color_hex(TEXT_COLOR_DEFAULT), 0);
            break;

        case SETTING_POWER_WARNING_TYPE_BOTH:
            if (low) {
                if (beep_gap++ == BEEP_INTERVAL) {
                    beep();
                    beep_gap = 0;
                }
                lv_obj_set_style_text_color(label[STS_BATT], lv_color_make(255, 0, 0), 0);
            } else
                lv_obj_set_style_text_color(label[STS_BATT], lv_color_hex(TEXT_COLOR_DEFAULT), 0);
            break;
        default:
            break;
        }
    }

    static int hdzero_channel_last = 0;
    static int analog_channel_last = 0;
    static source_t source_last = SOURCE_HDZERO;
    static setting_sources_hdzero_band_t hdzero_band_last = SETTING_SOURCES_HDZERO_BAND_RACEBAND;
    uint8_t channel_changed = (hdzero_channel_last != g_setting.scan.channel) || (analog_channel_last != g_setting.source.analog_channel);
    if (channel_changed || (source_last != g_source_info.source) || (hdzero_band_last != g_setting.source.hdzero_band)) {
        memset(buf, 0, sizeof(buf));
        if (g_source_info.source == SOURCE_HDZERO)
            sprintf(buf, "RF: HDZero %s", channel2str(1, g_setting.source.hdzero_band, g_setting.scan.channel & 0x7F));
        else if (g_source_info.source == SOURCE_HDMI_IN)
            sprintf(buf, "HDMI In");
        else if (g_source_info.source == SOURCE_AV_IN)
            sprintf(buf, "AV In");
        else if (g_source_info.source == SOURCE_AV_MODULE)
            sprintf(buf, "RF: Analog %s", channel2str(0, 0, g_setting.source.analog_channel));
        else
            sprintf(buf, " ");

        lv_label_set_text(label[STS_SOURCE], buf);
    }
    hdzero_channel_last = g_setting.scan.channel;
    analog_channel_last = g_setting.source.analog_channel;
    source_last = g_source_info.source;
    hdzero_band_last = g_setting.source.hdzero_band;

    if (page_storage_is_sd_repair_active()) {
        lv_img_set_src(img_sdc, &img_sdcard);
        lv_label_set_text(label[STS_SDCARD], "Integrity check");
    } else {
        if (g_sdcard_enable) {
            int cnt = get_videofile_cnt();
            float gb = sdcard_free_size() / 1024.0;
            lv_img_set_src(img_sdc, &img_sdcard);
            if (cnt != 0) {
                if (sdcard_is_full())
                    snprintf(buf, sizeof(buf), "%d %s, %s %s", cnt, "clip(s)", "SD Card", "full");
                else
                    snprintf(buf, sizeof(buf), "%d %s, %.2fGB %s", cnt, "clip(s)", gb, "available");
            } else {
                if (sdcard_is_full())
                    snprintf(buf, sizeof(buf), "#FF0000 %s %s#", "SD Card", "full");
                else
                    snprintf(buf, sizeof(buf), "%.2fGB %s", gb, "available");
            }
        } else {
            lv_img_set_src(img_sdc, &img_noSdcard);

            if (sdcard_inserted()) {
                sprintf(buf, "Unsupported");
            } else {
                sprintf(buf, "No SD Card");
            }
        }

        lv_label_set_text(label[STS_SDCARD], buf);
    }

    if (g_setting.is_pro) {
        if (g_setting.elrs.enable)
            lv_label_set_text(label[STS_ELRS], "ELRS: On ");
        else
            lv_label_set_text(label[STS_ELRS], "ELRS: Off");

        page_wifi_get_statusbar_text(buf, sizeof(buf));
        lv_label_set_text(label[STS_WIFI], buf);
    }
}
