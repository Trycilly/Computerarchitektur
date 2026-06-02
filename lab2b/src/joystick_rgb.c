/* Done by Dominic Sehorz */

/**
 * \file Make the Joystick steer the RGB-LED.
 * The Joystick has two potentionmeter for the X- and the Y-axis.
 * These may be read out using the GPIO pins that support converting
 * voltages into digital values using the ADC component with it's
 * 12-bit quantization using successive-approximation ADC (SAR).
 *
 * Read out the X-axis to color-code the RGB LED in a loop.
 * 
 * With Pulse-Width-Modulation (PWM) one may define a duty-cycle
 * from 0 to 100% (0 to 254) on a GPIO initialized to output using
 * a digital signal with PWM. Here the actual timing is not relevant
 * (in comparison to servo-motors and alike).
 * You shall light the PWM from RED to BLUE: 100% RED when the X-axis
 * is at the maximum deflection, and 100% BLUE when the X-axis is at 
 * the lowest/minimum deflection.
 * Instead of any color-correction / brightness-correction, just linearly
 * scale between those two value, as in BLUE=100-RED.
 */
#include <stdio.h>
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#define GPIO_X         26
#define GPIO_Y         27
#define GPIO_LED_RED    5
#define GPIO_LED_GREEN  3
#define GPIO_LED_BLUE   4

int main() {

    /** TODO: Initialize ADC to read out  */
    // Initialize ADC hardware
    adc_init();

    // Initialize GPIO pins for ADC usage
    adc_gpio_init(GPIO_X);
    adc_gpio_init(GPIO_Y);

    // Select ADC input channel 0 (GPIO26 = ADC0)
    adc_select_input(0);
    /** END OF TODO */

    /** TODO: Initialize PWM for the LEDs  */
    // Set LED GPIOs as output
    gpio_init(GPIO_LED_RED);
    gpio_set_dir(GPIO_LED_RED, GPIO_OUT);

    gpio_init(GPIO_LED_BLUE);
    gpio_set_dir(GPIO_LED_BLUE, GPIO_OUT);
    /** END OF TODO */
    
    while (true)
    {
        uint16_t x;
        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / ((1 << 12) - 1);
        
        /** TODO: Read out the joysticks X-Value */
        // Read current ADC value from joystick X-axis
        x = adc_read();
        /** END OF TODO */

        /** TODO: Change the PWM-Level from 100% RED at maximum X - Level to 100% BLUE at minium  */
        // Scale 12-bit ADC value (0-4095) to PWM range (0-255)
        uint8_t red_pwm = (x * 255) / 4095;

        // BLUE is inverse of RED
        uint8_t blue_pwm = 255 - red_pwm;

        // Simple software PWM simulation:

        for (int frame = 0; frame < 2000; frame++) {
            for (int i = 0; i < 255; i++) {
        gpio_put(GPIO_LED_RED,  !(i < red_pwm));
        gpio_put(GPIO_LED_BLUE, !(i < blue_pwm));
        gpio_put(GPIO_LED_GREEN, 1);
        sleep_us(1);
            }
        }
        /** END OF TODO */

        printf("Raw joystick value: x:%4d (%1.3f V)\n",
               x, x * conversion_factor);
        // sleep_ms(500);
    }
}
