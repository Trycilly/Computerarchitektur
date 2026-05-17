/**
 * @file Register a callback to the GPIO of a button, which You need to
 * initialized to either a external pull-down or an internal pull-up.
 *
 * This button is not debounced (no RC or RL filter in hardware).
 * Therefore You will need to debounce in software, using Pico's
 * low-level ticks interface or Pico's high-level time functions.
 *
 * The loop must count correctly the number of button presses.
 * Any button press > 200ms should be registered, any event shorter
 * than that filtered:
 * - Filtered button events should trigger light up only the RED LED
 * - Normal button presses should trigger light up only the GREEN LED
 * - Long button presses (>1s) should trigger light up only the BLUE LED
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define GPIO_BUTTON    0
#define GPIO_LED_RED   3
#define GPIO_LED_GREEN 4
#define GPIO_LED_BLUE  5

int button_press = 0;
int last_button_press = 0;

/** TODO: You will need more data to communicate from Interrupt handler gpio_callback */
// Timestamp when button was pressed down
absolute_time_t press_start_time;

// Timestamp of last valid interrupt (for debounce)
absolute_time_t last_interrupt_time;

// Stores last interrupt type for debounce
uint32_t last_event = 0;

// Debounce threshold in milliseconds
#define DEBOUNCE_MS 50
/** END OF TODO */

void gpio_callback(uint gpio, uint32_t events)
{
    /** TODO: Program filter routine for the evenst */
    // Current timestamp
    absolute_time_t now = get_absolute_time();

    /* Debounce: ignore ANY event within 50ms */
    if (absolute_time_diff_us(last_interrupt_time, now) < (DEBOUNCE_MS * 1000))
    {
        return;
    }

    last_interrupt_time = now;

    // Save timestamp of this interrupt
    last_interrupt_time = now;
    last_event = events;

    // Rising edge = button pressed
    if (events & GPIO_IRQ_EDGE_RISE)
    {
        // Save time when button press started
        press_start_time = now;
    }

    // Falling edge = button released
    if (events & GPIO_IRQ_EDGE_FALL)
    {
        // Calculate press duration in milliseconds
        int64_t press_duration =
            absolute_time_diff_us(press_start_time, now) / 1000;

        // Turn off all LEDs first
        gpio_put(GPIO_LED_RED, 0);
        gpio_put(GPIO_LED_GREEN, 0);
        gpio_put(GPIO_LED_BLUE, 0);

        // Filtered event (< 200ms)
        if (press_duration < 200)
        {
            gpio_put(GPIO_LED_RED, 1);
        }

        // Normal press (200ms - 1000ms)
        else if (press_duration <= 1000)
        {
            gpio_put(GPIO_LED_GREEN, 1);

            // Count valid button press
            button_press++;
        }

        // Long press (> 1000ms)
        else
        {
            gpio_put(GPIO_LED_BLUE, 1);

            // Count valid button press
            button_press++;
        }
    }
    /** END OF TODO */
}

void main(void)
{
    stdio_init_all();

    /** TODO: Fill in Initialization */
    // Initialize button GPIO
    gpio_init(GPIO_BUTTON);
    gpio_set_dir(GPIO_BUTTON, GPIO_IN);

    // Enable internal pull-down resistor
    // (safe even if external pull-down already exists)
    gpio_pull_down(GPIO_BUTTON);

    // Initialize RGB LEDs
    gpio_init(GPIO_LED_RED);
    gpio_set_dir(GPIO_LED_RED, GPIO_OUT);

    gpio_init(GPIO_LED_GREEN);
    gpio_set_dir(GPIO_LED_GREEN, GPIO_OUT);

    gpio_init(GPIO_LED_BLUE);
    gpio_set_dir(GPIO_LED_BLUE, GPIO_OUT);

    // Turn all LEDs off initially, if inverted: switch 0 with 1
    gpio_put(GPIO_LED_RED, 0);
    gpio_put(GPIO_LED_GREEN, 0);
    gpio_put(GPIO_LED_BLUE, 0);

    // Initialize interrupt timestamp
    last_interrupt_time = get_absolute_time();

    // Register GPIO interrupt callback
    gpio_set_irq_enabled_with_callback(
        GPIO_BUTTON,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        &gpio_callback
    );
    /** END OF TODO */

    // Keep the busy loop outputting
    while (true)
    {
        if (last_button_press != button_press)
        {
            printf("Number of times button pressed:%d\n", button_press);
            last_button_press = button_press;
        }
        sleep_ms (20);
    }
}
