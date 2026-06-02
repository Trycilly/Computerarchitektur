    /**
     * @file Implements the cursor allowing to set the time:
     *  - when moving the Joystick, a cursor (two lines) should appear
     *    (see clock_gui.c)
     *  Using the joystack, one shall edit (change) the hour and minute:
     *  - when not in editing mode:
     *    - Pulling the joystick position of the cursor should change:
     *       pull up: decrease y  /  pull down: increase y
     *       pull left: decrease x / pull right: increase x
     *    - when clicking on hour / minute: Select Edit mode for hour or minute
     *  - when in editing mode:
     *    - Pulling the joystick should change the hour / minute:
     *       pull up: increase hour or minute / pull down: decrease hour or minute
     *    - When clicking: Get out of Editing mode
     */
    #include <stdio.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <hardware/gpio.h>
    #include <hardware/adc.h>

    #include "clock.h"
    #include "clock_time.h"
    #include "clock_cursor.h"

    #define CURSOR_VISIBLE_TIME     (5 * 1000 * 1000) // 5 seconds
    #define CURSOR_BUTTON_PRESS_TIME (200 * 1000)      // 200 ms debounce

    /*
    * The cursor's current state (position, edit mode, visibility,
    * and when the cursor was last moved).
    */
    static pos_t cursor_pos = {120, 120};

    static bool     cursor_visible = false;
    static uint64_t last_move_time = 0;

    static clock_cursor_edit_t cursor_edit = CURSOR_EDIT_NONE;

    static box_t hour_box;
    static box_t min_box;

    /* ------------------------------------------------------------------ */

    void clock_cursor_init(box_t box_hour, box_t box_minute)
    {
        hour_box = box_hour;
        min_box  = box_minute;

        cursor_visible = false;
        cursor_edit    = CURSOR_EDIT_NONE;
        last_move_time = 0;

        /* ADC for joystick X/Y axes */
        adc_init();
        adc_gpio_init(JOYSTICK_GPIO_X);
        adc_gpio_init(JOYSTICK_GPIO_Y);

        /* Button GPIO: input with pull-up (button pulls to GND when pressed) */
        gpio_init(JOYSTICK_GPIO_BUTTON);
        gpio_set_dir(JOYSTICK_GPIO_BUTTON, GPIO_IN);
        gpio_pull_up(JOYSTICK_GPIO_BUTTON);
    }

    /* ------------------------------------------------------------------ */

    /*
    * Convert ADC readout to a cursor movement delta.
    * Dead-zone in the centre, two speed levels.
    */
    static int16_t adc_to_pos_value(uint16_t adc)
    {
        int16_t centered = (int16_t)adc - 2048;
        if (centered > 1800)  return  2;
        if (centered > 200)  return  1;
        if (centered < -1800) return -2;
        if (centered < -200) return -1;
        return 0;
    }

    /*
    * Convert ADC readout to a time-change delta.
    * Dead-zone in the centre, two speed levels.
    */
    static int16_t adc_to_timechange_value(uint16_t adc)
    {
        int16_t centered = (int16_t)adc - 2048;

        if (centered >  1500) return  2;
        if (centered >  500) return  1;
        if (centered < -1500) return -2;
        if (centered < -500) return -1;
        return 0;
    }

    /* ------------------------------------------------------------------ */

    /*
    * Helper: return true if pos is inside box.
    */
    static bool pos_in_box(pos_t pos, box_t box)
    {
        return (pos.x >= box.upper_left.x  && pos.x <= box.lower_right.x &&
                pos.y >= box.upper_left.y  && pos.y <= box.lower_right.y);
    }

    /* ------------------------------------------------------------------ */

    bool clock_cursor_update(uint64_t tick_us)
    {
        bool changed = false;

        /* --- Fade out after CURSOR_VISIBLE_TIME of inactivity --- */
        if (cursor_visible && (tick_us - last_move_time > CURSOR_VISIBLE_TIME)) {
            cursor_visible = false;
            cursor_edit    = CURSOR_EDIT_NONE;
            return true; // GUI must redraw to remove cursor
        }

        /* --- Button handling (debounced, edge-triggered on release) --- */
        static bool     button_was_pressed  = false;
        static uint64_t button_press_time   = 0;
        bool button_pressed = !gpio_get(JOYSTICK_GPIO_BUTTON); // active-low

        if (button_pressed && !button_was_pressed) {
            /* Falling edge: record press time */
            button_press_time  = tick_us;
            button_was_pressed = true;
        }
        else if (!button_pressed && button_was_pressed) {
            /* Rising edge: button released */
            button_was_pressed = false;

            if ((tick_us - button_press_time) >= CURSOR_BUTTON_PRESS_TIME) {
                if (cursor_edit != CURSOR_EDIT_NONE) {
                    /* Any click in edit mode → exit edit mode */
                    cursor_edit = CURSOR_EDIT_NONE;
                }
                else if (cursor_visible) {
                    /* Click in normal mode → enter edit mode if over a box */
                    if (pos_in_box(cursor_pos, hour_box))
                        cursor_edit = CURSOR_EDIT_HOUR;
                    else if (pos_in_box(cursor_pos, min_box))
                        cursor_edit = CURSOR_EDIT_MINUTE;
                }

                /* Any click keeps the cursor visible and resets the timer */
                cursor_visible = true;
                last_move_time = tick_us;
                changed = true;
            }
        }

        /* --- Read joystick axes --- */
        adc_select_input(0);
        uint16_t adc_x = adc_read();

        adc_select_input(1);
        uint16_t adc_y = adc_read();

        /* --- Joystick logic --- */
        if (cursor_edit == CURSOR_EDIT_NONE) {
            /* Normal mode: move the cursor crosshair */
            int16_t dx = adc_to_pos_value(adc_x);
            int16_t dy = adc_to_pos_value(adc_y);

            if (dx != 0 || dy != 0) {
                cursor_pos.x += dx;
                cursor_pos.y += dy;

                /* Clamp to display bounds */
                if (cursor_pos.x <   0) cursor_pos.x =   0;
                if (cursor_pos.x > 239) cursor_pos.x = 239;
                if (cursor_pos.y <   0) cursor_pos.y =   0;
                if (cursor_pos.y > 239) cursor_pos.y = 239;

                cursor_visible = true;
                last_move_time = tick_us;
                changed = true;
            }
        }
        else {
        /* Edit mode: joystick Y changes hour or minute */
        int16_t change = adc_to_timechange_value(adc_y);
        
        // Timer to track when the las adjustment was made
        static uint64_t last_edit_time = 0;

        if (change != 0) {
            // Adjust speed here (Time in microseconds):
            // Lightly pressed  (1 or -1) -> Wait 500ms until the next step
            // Frimly pressed  (2 or -2) -> Wait 150ms (fast scrolling)
            uint64_t speed_delay = (change == 2 || change == -2) ? (150 * 1000) : (500 * 1000);

            if (tick_us - last_edit_time >= speed_delay) {
                
                // Force the step to 1 or -1 so that the numbers 
                // do not skip by intervals of two during fast scrolling
                int16_t step = (change > 0) ? 1 : -1;

                if (cursor_edit == CURSOR_EDIT_HOUR)
                    clock_time_change_hour_utc(step);
                else
                    clock_time_change_minute_utc(step);

                /* Keep cursor alive while actively editing */
                last_edit_time = tick_us;
                last_move_time = tick_us;
                changed = true;
            }
        }
        else {
            // If joystick is centered, reset immediately.
            // This ensures the next "tap" reacts without any delay.
            last_edit_time = 0;
        }
    }
        return changed;
    }

    /* ------------------------------------------------------------------ */

    clock_cursor_edit_t clock_cursor_get_state(pos_t *pos, bool *visible)
    {
        if (pos     != NULL) { pos->x = cursor_pos.x; pos->y = cursor_pos.y; }
        if (visible != NULL) { *visible = cursor_visible; }
        return cursor_edit;
    }
