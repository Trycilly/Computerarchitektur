/**
 * \file hexToASCII.S
 * \brief Convert a 32-bit value to a NUL-terminated ASCII hex string.
 *
 * The caller passes the destination pointer in a0 and the value in a1.
 * The result is always formatted as "0x1234ABCD" with exactly 8 hex digits.
 */

        .section const_section, "a"
        .align  2
        .global H2A
H2A:
        .asciz  "0123456789ABCDEF"

        .text
        .align 1
        .global hexToASCII
hexToASCII:
        mv      t0, a0                # t0 = destination pointer
        sb      zero, 0(t0)           # clear first byte temporarily if needed
        li      t1, '0'
        sb      t1, 0(t0)
        li      t1, 'x'
        sb      t1, 1(t0)
        addi    t0, t0, 2             # point to the first hex digit
        mv      t2, a1                # t2 = value to convert
        li      t3, 28                # bit shift for highest nibble
        li      t4, 0xf
.hex_loop:
        srl     t5, t2, t3            # extract nibble
        andi    t5, t5, t4
        la      t6, H2A               # base of hex lookup table
        add     t6, t6, t5
        lbu     t7, 0(t6)
        sb      t7, 0(t0)
        addi    t0, t0, 1
        addi    t3, t3, -4
        bgez    t3, .hex_loop
        sb      zero, 0(t0)           # terminating NUL
        ret
