/**
 * @file Implements the main loop initializing components
 */
#include "pico/stdlib.h"
#include "Config/DEV_Config.h"
#include "hardware/ticks.h"
#include "hardware/gpio.h" // Wichtig für die direkte GPIO-Steuerung

#include "clock.h"
#include "clock_cursor.h"
#include "clock_gui.h"
#include "clock_time.h"

/**
 * @file Implements the main loop initializing components
 */
#include "pico/stdlib.h"
#include "Config/DEV_Config.h"
#include "hardware/ticks.h"
#include "hardware/gpio.h"

#include "clock.h"
#include "clock_cursor.h"
#include "clock_gui.h"
#include "clock_time.h"

#define NORMAL_BUTTON_GPIO  0
#define BUTTON_DEBOUNCE_US  (200 * 1000) // 200ms

static timezones_t current_timezone = TIMEZONE_UTC;

int main()
{
    static uint64_t tick_us;

    if (DEV_Module_Init() != 0)
        ERROR("DEV_Module_Init", -1);

    // Normal button: cycles timezones (active-low)
    gpio_init(NORMAL_BUTTON_GPIO);
    gpio_set_dir(NORMAL_BUTTON_GPIO, GPIO_IN);
    gpio_pull_up(NORMAL_BUTTON_GPIO);

    clock_gui_init();
    clock_gui_update(0); // initial draw

    box_t box_hour   = { {90,  175}, {110, 205} };
    box_t box_minute = { {115, 175}, {165, 205} };
    clock_cursor_init(box_hour, box_minute);

    static bool     tz_btn_was_pressed = false;
    static uint64_t tz_btn_press_time  = 0;
    static int      current_tz         = TIMEZONE_UTC;


    while (true) {
        bool update_gui = false;
        tick_us = time_us_64();

        // Update cursor (movement + fade)
        if (clock_cursor_update(tick_us))
            update_gui = true;

        // Update time (1 Hz)
        if (clock_time_inc_second(tick_us))
            update_gui = true;

        // Keep GUI updating while in edit mode
        clock_cursor_edit_t edit = clock_cursor_get_state(NULL, NULL);
        if (edit != CURSOR_EDIT_NONE)
            update_gui = true;

        // Normal button: cycle timezone (debounced, Normal Mode only)
        bool tz_btn = !gpio_get(NORMAL_BUTTON_GPIO); // active-low

        if (tz_btn && !tz_btn_was_pressed) {
            tz_btn_press_time  = tick_us;
            tz_btn_was_pressed = true;
        }
        else if (!tz_btn && tz_btn_was_pressed) {
            tz_btn_was_pressed = false;
            if ((tick_us - tz_btn_press_time) >= BUTTON_DEBOUNCE_US) {
                if (edit == CURSOR_EDIT_NONE) {
                    current_tz = (current_tz + 1) % TIMEZONE_COUNT;
                    clock_time_set_timezone((timezones_t)current_tz);
                    update_gui = true;
                }
            }
        }

        if (update_gui)
            clock_gui_update(tick_us);
    }

    
    DEV_Module_Exit();
    return 0;
    
}