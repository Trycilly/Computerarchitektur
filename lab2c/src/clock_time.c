/* Done by Alen Osmanagic */

/**
 * @file Time (and later date?) handling
 */
#include <pico/stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "clock_time.h"

/********************** Type definitions **********************/
typedef struct timezone_def
{
    char name_short[8];
    char name[32];
    int hour_offset_UTC;
} timezone_def_t;

/********************** Local variables **********************/
static int seconds = CLOCK_TIME_START_SECOND;
static int minutes = CLOCK_TIME_START_MINUTE;
static int hours = CLOCK_TIME_START_HOUR;

/* Timezone settings and tracking */
static int timezone_offset_hours = CLOCK_TIME_START_OFFSET_HOUR;
static uint64_t last_tick_us = 0;

static timezones_t active_timezone = TIMEZONE_UTC;

/* Lookup table for supported timezones */
static timezone_def_t timezone_def[] = {
    {"UTC", "Coordinated Universal Time", 0},
    {"WET", "Western European Time", 0},
    {"WEST", "Western European Summer Time", 1},
    {"EST", "Eastern Standard Time", -5}};

const char *clock_time_get_timezone_name(void)
{
    return timezone_def[active_timezone].name_short;
}

/** Robust wrap logic handling any arbitrary change values */
static int wrap24(int h)
{
    h = h % 24;
    if (h < 0) /* Handle negative modulo for backward adjustments */
        h += 24;
    return h;
}

static int wrap60(int v)
{
    v = v % 60;
    if (v < 0) /* Handle negative modulo for backward adjustments */
        v += 60;
    return v;
}

bool clock_time_inc_second(uint64_t tick_us)
{
    /* Initialize baseline timestamp on the first call */
    if (last_tick_us == 0)
        last_tick_us = tick_us;

    /* Return early if a full second hasn't passed yet (1,000,000 microseconds) */
    if ((tick_us - last_tick_us) < (1000 * 1000))
        return false;

    last_tick_us += (1000 * 1000);

    /* Cascade time updates from seconds up to hours */
    seconds++;
    if (seconds >= 60)
    {
        seconds = 0;
        minutes++;
        if (minutes >= 60)
        {
            minutes = 0;
            hours++;
            if (hours >= 24)
            {
                hours = 0;
            }
        }
    }
    return true;
}

void clock_time_set_utc(int hour, int minute, int second)
{
    assert(hour >= 0 && hour < 24);
    assert(minute >= 0 && minute < 60);
    assert(second >= 0 && second < 60);
    hours = hour;
    minutes = minute;
    seconds = second;
}

void clock_time_get_utc(int *hour, int *minute, int *second)
{
    assert(hour != NULL);
    assert(minute != NULL);
    assert(second != NULL);

    *hour = hours;
    *minute = minutes;
    *second = seconds;
}

void clock_time_get_local(int *hour, int *minute, int *second)
{
    assert(hour != NULL);
    assert(minute != NULL);
    assert(second != NULL);

    /* Calculate local hour by applying the active timezone offset */
    *hour = wrap24(hours + timezone_offset_hours);
    *minute = minutes;
    *second = seconds;
}

void clock_time_set_timezone(timezones_t tz)
{
    assert(tz >= 0 && tz < TIMEZONE_COUNT);
    active_timezone = tz;
    timezone_offset_hours = timezone_def[tz].hour_offset_UTC;
}

void clock_time_change_hour_utc(int change_value)
{
    hours = wrap24(hours + change_value);
}

void clock_time_change_minute_utc(int change_value)
{
    minutes = wrap60(minutes + change_value);
}

/**
 * Visualizer function to swap formatting between 12-hour and 24-hour systems
 * Pass a destination char buffer with a minimum size of 12 bytes.
 */
void clock_time_format_string(char *buffer, int hour, int minute, int second)
{
    assert(buffer != NULL);

#if SELECT_12HOURS == 1
    /* Convert 24-hour format to 12-hour AM/PM format */
    int hour_12 = hour % 12;
    if (hour_12 == 0)
        hour_12 = 12;
    const char *am_pm = (hour >= 12) ? "PM" : "AM";
    snprintf(buffer, 16, "%02d:%02d:%02d %s", hour_12, minute, second, am_pm);
#else
    snprintf(buffer, 16, "%02d:%02d:%02d", hour, minute, second);
#endif
}

#if SELECT_12HOURS == 1
/**
 * @brief Check if current time is in PM (12:00-23:59)
 * @return true if PM, false if AM
 */
bool clock_time_is_pm(void)
{
    return (hours >= 12);
}
#endif
