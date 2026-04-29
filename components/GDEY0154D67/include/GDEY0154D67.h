#pragma once

#include <cstdint>

#ifndef ENABLE_DISPLAY_LVGL
#define ENABLE_DISPLAY_LVGL 1
#endif

#if ENABLE_DISPLAY_LVGL
#include <lvgl.h>
#endif

enum class GDEY0154D67_Orientation
{
    ORIENTATION_0 = 0,
    ORIENTATION_90 = 90,
    ORIENTATION_180 = 180,
    ORIENTATION_270 = 270
};

void GDEY0154D67_init();
void GDEY0154D67_set_orientation(GDEY0154D67_Orientation orientation);
bool GDEY0154D67_is_initialized();

void GDEY0154D67_clear_screen();
void GDEY0154D67_black_screen();
void GDEY0154D67_refresh();

#if ENABLE_DISPLAY_LVGL
void GDEY0154D67_draw_partial(const lv_area_t *area, lv_color_t *color_p);
#endif
