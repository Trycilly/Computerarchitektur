/**
 * \file decToASCII.S
 * \brief Convert a 32-bit signed integer to a fixed-width ASCIIZ decimal string.
 *
 * The caller passes the destination pointer in a0 and the value in a1.
 * The result is always 10 digits right-aligned with sign at position 0.
 */

         .text
        .align 1
        .global decToASCII

/**
 * Converts a 32-bit signed integer into a null-terminated decimal string.
 * Format: "[sign][10 digits]"
 * a0 = destination string
 * a1 = value to convert
 */

decToASCII:
        mv      t0, a0                                  # t0 = output string pointer
        mv      t1, a1                                  # t1 = input value

        /** Handle sign of the number */
        bgez    t1, .positive                           # if value >= 0 → positive case

        li      t2, '-'                                 # load minus sign
        sb      t2, 0(t0)                               # store '-' in first byte
        addi    t0, t0, 1                               # move pointer forward

        /** Convert negative value safely (two's complement handling) */
        not     t1, t1                                  # bitwise NOT of value
        addi    t1, t1, 1                               # +1 → absolute value (safe for INT_MIN)
        j       .digits                                 # jump to digit processing

.positive:
        li      t2, ' '                                 # load space character
        sb      t2, 0(t0)                               # store space as sign
        addi    t0, t0, 1                               # advance output pointer

        /** Start digit extraction */
.digits:
        li      t3, 1000000000                          # initial divisor (10^9)
        li      t4, 10                                  # number of digits to process

        /** Loop over all decimal digit positions */
.loop:
        div     t5, t1, t3                              # t5 = quotient (digit)
        rem     t1, t1, t3                              # t1 = remainder

        addi    t5, t5, '0'                             # convert digit to ASCII
        sb      t5, 0(t0)                               # store ASCII digit
        addi    t0, t0, 1                               # advance output pointer

        li      t6, 10                                  # divisor base
        div     t3, t3, t6                              # reduce divisor by factor 10

        addi    t4, t4, -1                              # decrement digit counter
        bnez    t4, .loop                               # repeat until all digits processed

        /** Null-terminate the string */
        sb      zero, 0(t0)                             # write string terminator '\0'
        ret                                             # return to caller
