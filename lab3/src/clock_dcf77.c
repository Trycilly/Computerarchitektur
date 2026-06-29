/**
 * @file Implements the sampling and parsing of the DCF77 signal.
 *  - Setup the GPIO pin 13 as input -- don't forget to turn of pull-ups/pull-downs,
 *    when pin is drawn low.
 *  - Create FreeRTOS tasks for sampling (high priority) the signal and decoding the signal (lower priority),
 *  - Create a FreeRTOS queue to exchange the data between these tasks.
 *    Have the decoding task set the time in the clock_time module, if a valid signal is received.
 */
#include <pico/stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "clock.h"
#include "clock_dcf77.h"
#include "clock_time.h"

static const int DCF77_PULSE_THRESHOLD_MIN_LOW = 180;
static const int DCF77_PULSE_THRESHOLD_MIN_HIGH = 220;
static const int DCF77_PULSE_THRESHOLD_SEC_LOW = 90;
static const int DCF77_PULSE_THRESHOLD_SEC_HIGH = 150;

static const int bcd_1s_weights[4] = {1, 2, 4, 8};
static const int bcd_10s_weights[4] = {10, 20, 40, 80};


static TaskHandle_t clock_dcf77_handle_sample = NULL;
static QueueHandle_t clock_dcf77_queue;


typedef struct
{
    uint8_t bits[59];
    int bit_index;
} DCF77_Bitarray;

static DCF77_Bitarray dcf77_bitarray;

/* XXX: BEGIN THIS MAY BE DELETED */
static int DCF77_flip_century_count = 0;
static int DCF77_start_year = DCF77_ASSUME_START_YEAR;
/* XXX: END THIS MAY BE DELETED */

/**  TODO: BEGIN delete const int definition */
static const int GPIO_RGB_LED_MONO = 1;
static const int GPIO_RGB_LED_R = 3;
static const int GPIO_RGB_LED_G = 4;
static const int GPIO_RGB_LED_B = 5;
/** TODO: END delete const int definition */

static void dcf77_init(DCF77_Bitarray *bitarray)
{
    assert(NULL != bitarray);

    bitarray->bit_index = 0;
    for (int i = 0; i < 59; ++i)
        bitarray->bits[i] = 0;
}

static void dcf77_reset(DCF77_Bitarray *bitarray)
{
    assert(NULL != bitarray);
    dcf77_init(bitarray);
}


/**
 * Parse the bitarray and if successful return into time.
 *
 * @param bitarray [in]    the 59 bits gathered
 * @param time [out]       the decoded time and date with further info
 *
 * @return false           once parsing failed due to parity errors, logical errors
 * @return true            if parsing was successfull and time is valid
 */
static int bcd_to_int(const uint8_t *bits, int start, int length) {
    int val = 0;
    int multiplier = 1;
    for (int i = 0; i < length; ++i) {
        if (i == 4) multiplier = 10; // Switch to 10s place after 4 bits
        val += bits[start + i]  * ((i < 4) ? bcd_1s_weights[i] : bcd_10s_weights[i - 4]);
    }
    return val;
}

static bool check_parity(const uint8_t *bits, int start, int end, uint8_t parity_bit) {
    int sum = 0;
    for (int i = start; i <= end; i++) {
        sum += bits[i];
    }
    return (sum % 2) == parity_bit;
}

static bool dcf77_parse_frame(const DCF77_Bitarray *bitarray, clock_dcf77_time_t *time)
{
    assert(bitarray != NULL);
    assert(time != NULL);

    //Start-Bit (Bit 20 must be 1)
    if (bitarray->bits[20] != 1) return false;
    if (!check_parity(bitarray->bits, 21, 27, bitarray->bits[28])) return false; // P1 (Minutes)
    if (!check_parity(bitarray->bits, 29, 34, bitarray->bits[35])) return false; // P2 (Hours)
    if (!check_parity(bitarray->bits, 36, 57, bitarray->bits[58])) return false; // P3 (Date)

    // Decode the time and date information from the bitarray
    time->minute = bcd_to_int(bitarray->bits, 21, 7);
    time->hour = bcd_to_int(bitarray->bits, 29, 6);
    time->day = bcd_to_int(bitarray->bits, 36, 6);
    time->month = bcd_to_int(bitarray->bits, 45, 5);
    time->year = bcd_to_int(bitarray->bits, 50, 8) + DCF77_ASSUME_START_YEAR;

    if (time->hour > 23 || time->minute > 59 || time->day < 1 || time->day > 31 || time->month > 12) {
        return false; // Invalid time values
    }

    time->valid = true;
    return true;
}

static inline void dcf77_feed_event(DCF77_Bitarray *bitarray, clock_dcf77_event_t event)
{
    assert(bitarray != NULL);

    /** TODO: BEGIN: Delete Analysing the seconds pulse */
    if (event == VALID_ZERO  || event == VALID_ONE) {
        if (bitarray -> bit_index < 59) {
            bitarray -> bits[bitarray -> bit_index] = (event == VALID_ONE) ? 1 : 0;
            // RGB LED Update: Blue for 0 Green for 1
            gpio_put(GPIO_RGB_LED_R, 0);
            gpio_put(GPIO_RGB_LED_G, (event == VALID_ONE) ? 1 : 0);
            gpio_put(GPIO_RGB_LED_B, (event == VALID_ZERO) ? 1 : 0);
        }
    }
    else if (event == VALID_SECOND) {
        bitarray -> bit_index = 0; // Reset bit index at the start of a new frame
        // RGB LED Update: Yellow for second pulse
        gpio_put(GPIO_RGB_LED_MONO, !gpio_get(GPIO_RGB_LED_MONO)); // Toggle Mono LED for second pulse
    }
    else if (event == VALID_MINUTE) {
        clock_dcf77_time_t decoded_time;
        if (bitarray -> bit_index >= 58 && dcf77_parse_frame(bitarray, &decoded_time)) {
            // Set the decoded time in the clock_time module
            clock_time_set_from_dcf77(decoded_time.hour, decoded_time.minute, decoded_time.day, decoded_time.month, decoded_time.year);
            gpio_put(GPIO_RGB_LED_R, 0);
        }
        else {
            // Error! Turn on red LED for invalid frame
            gpio_put(GPIO_RGB_LED_R, 1);
            gpio_put(GPIO_RGB_LED_G, 0);
            gpio_put(GPIO_RGB_LED_B, 0);
        }
        dcf77_reset(bitarray); // Reset bitarray for the next frame
    }
    else if (event == INVALID) {
        gpio_put(GPIO_RGB_LED_R, 1); // Turn on red LED for invalid pulse
    }
    /** TODO: END: Delete Analysing the seconds pulse */
}


static void DCF77_sample_fn(void *params)
{
    const TickType_t xDelay = pdMS_TO_TICKS(CLOCK_DCF77_SAMPLE_INTERVAL_DELAY_MS);
    static TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    /** TODO: BEGIN Initialize sampling the values */
    static bool last_level = true;
    static uint32_t last_edge_time_ms = 0;
    static uint32_t last_falling_edge_ms = 0;

    while (true)
    {
        // Pin invertiert lesen (H = 0, L = 1 laut Aufgabe)
        bool current_level = gpio_get(DCF77_IO) == 0; 
        uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

        if (current_level != last_level)
        {
            uint32_t duration = current_time_ms - last_edge_time_ms;
            last_edge_time_ms = current_time_ms;

            if (current_level == false) // Positive Flanke (Low -> High)
            {
                // T_Low messen
                if (duration >= DCF77_PULSE_THRESHOLD_SEC_LOW && duration <= DCF77_PULSE_THRESHOLD_SEC_HIGH) {
                    dcf77_feed_event(&dcf77_bitarray, VALID_ZERO);
                } 
                else if (duration >= DCF77_PULSE_THRESHOLD_MIN_LOW && duration <= DCF77_PULSE_THRESHOLD_MIN_HIGH) {
                    dcf77_feed_event(&dcf77_bitarray, VALID_ONE);
                } else {
                    dcf77_feed_event(&dcf77_bitarray, INVALID);
                }
            }
            else // Negative Flanke (High -> Low)
            {
                uint32_t t_pulse = current_time_ms - last_falling_edge_ms;
                last_falling_edge_ms = current_time_ms;

                // T_Pulse messen (1s oder 2s)
                if (t_pulse >= 900 && t_pulse <= 1100) {
                    dcf77_feed_event(&dcf77_bitarray, VALID_SECOND);
                } 
                else if (t_pulse >= 1900 && t_pulse <= 2100) {
                    dcf77_feed_event(&dcf77_bitarray, VALID_MINUTE);
                } else {
                    dcf77_feed_event(&dcf77_bitarray, INVALID);
                }
            }
            last_level = current_level;
        }
        vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
    /** TODO: END Sampling the values */
}

bool clock_dcf77_init(void)
{
    int ret;

    dcf77_init(&dcf77_bitarray);

    // Initialize DCF77 IO as input with pull-up resistor, and set up an interrupt on the falling edge
    gpio_init(DCF77_IO);
    gpio_set_function(DCF77_IO, GPIO_FUNC_SIO);
    gpio_set_dir(DCF77_IO, GPIO_IN);
    gpio_disable_pulls(DCF77_IO);
    

    // Sample Task is the highest-priority Task and has configMAX_PRIORITIES-2 (one less than timer task)
    ret = xTaskCreate(DCF77_sample_fn, "DCF77_sample", 2*configMINIMAL_STACK_SIZE, NULL, CLOCK_DCF77_SAMPLE_TASK_PRIORITY, &clock_dcf77_handle_sample);
    if (ret != pdPASS)
        ERROR(ret, "xTaskCreate for DCF77_sample_fn returned != pdPASS");
    assert(clock_dcf77_handle_sample != NULL);
    // vTaskCoreAffinitySet(clock_dcf77_handle_sample, (0x1 << 1));

    return true;
}
