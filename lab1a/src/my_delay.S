/**
 * \file my_sleep_ms.S
 * \brief Function to sleep for ms multi-seconds.
 *
 * This is *not* a proper timing function, as it is just 
 * based on "experience" running on 150 MHz RP2350 with
 * this inner loop.  We will use the timers otherwise
 * for proper timing (just like the Pico-SDK employs).
 *
 * Author: Rainer Keller, HS-Esslingen
 */

        .text
        .align  1

# Define to a large enough value to denote one ms
.equ SLEEP_1_MS,   0xf
.equ SLEEP_MS,     500

/**
 * \fn void my_delay;
 * \brief Sleeps or rather busy-wait for 500 ms.
 * \return void
 */
        .global delay_0_5sec
delay_0_5sec:
        # Use only temporary registers so recursive calls remain safe
        li      t0, SLEEP_MS                                # Outer loop count for 500 ms chunks
.delay_outer:
        li      t1, SLEEP_1_MS                              # Inner loop count to waste cycles
.delay_inner:
        addi    t1, t1, -1
        bnez    t1, .delay_inner
        addi    t0, t0, -1
        bnez    t0, .delay_outer
        ret
