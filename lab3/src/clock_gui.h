#ifndef __CLOCK_GUI_H__
#define __CLOCK_GUI_H__

#include "lvgl/lvgl.h"

LV_IMAGE_DECLARE(clock_image);

void clock_gui_init(void);
void clock_gui_update(uint64_t tick_us);

#endif /* __CLOCK_GUI_H__ */