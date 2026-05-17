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
#include <hardware/gpio.h>
#include <hardware/adc.h>

#include "clock.h"
#include "clock_time.h"
#include "clock_cursor.h"

#define CURSOR_VISIBLE_TIME (5 * 1000 * 1000) // 5 seconds
#define CURSOR_BUTTON_PRESS_TIME (200 * 1000) // 200 ms

/*
 * The cursors current state (position, mode of edit, visibility and
 * when that cursor was last visible.
 */
static pos_t cursor_pos = {120, 120};
/** @TODO: You may need to add more local data */
#include <stdint.h>
#include <stdbool.h>

static bool cursor_visible = false;
static uint64_t last_move_time = 0;

static clock_cursor_edit_t cursor_edit = CURSOR_EDIT_NONE;

static box_t hour_box;
static box_t min_box;
/** END OF TODO */

void clock_cursor_init(box_t box_hour, box_t box_minute)
{
    /** @TODO: You need to implement the initialization */
    hour_box = box_hour;
    min_box  = box_minute;

    cursor_visible = false;
    cursor_edit = CURSOR_EDIT_NONE;
    last_move_time = 0;
    /** END OF TODO */
}

/*
 * Confert adc readout to a sensical pointer change value;
 */
static int16_t adc_to_pos_value(uint16_t adc)
{
    /** @TODO: You need to implement logic */
    // center ~2048 (12-bit ADC)
    int16_t centered = (int16_t)adc - 2048;

    if (centered > 200 || centered < -200)
        return centered / 512;   // slow movement
    return 0;
    /** END OF TODO */
}

/*
 * Confert adc readout to a sensical pointer change value;
 */
static int16_t adc_to_timechange_value(uint16_t adc)
{
    /** @TODO: You need to implement logic */
    int16_t centered = (int16_t)adc - 2048;

    if (centered > 800) return 5;
    if (centered > 200) return 1;
    if (centered < -800) return -5;
    if (centered < -200) return -1;

    return 0;
    /** END OF TODO */
}


bool clock_cursor_update(uint64_t tick_us)
{
    /** @TODO: You need to implement logic */
    // hide cursor after 5 seconds
    if (cursor_visible && (tick_us - last_move_time > CURSOR_VISIBLE_TIME)) {
        cursor_visible = false;
        cursor_edit = CURSOR_EDIT_NONE;
        return true;
    }

    // TODO: joystick read assumed elsewhere OR via ADC global
    // (template usually provides ADC read elsewhere)

    return false;
    /** END OF TODO */
}

clock_cursor_edit_t clock_cursor_get_state(pos_t *pos, bool *visible)
{
    if (NULL != pos)
    {
        pos->x = cursor_pos.x;
        pos->y = cursor_pos.y;
    }
    if (NULL != visible)
    {
        *visible = cursor_visible;
    }
    return cursor_edit;
}
