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
decToASCII:
        mv      t0, a0                # t0 = destination pointer
        mv      t1, a1                # t1 = value to convert
        bgez    t1, .positive
        li      t2, '-'
        sb      t2, 0(t0)
        sub     t1, zero, t1          # absolute value as unsigned for INT_MIN
        j       .sign_done
.positive:
        li      t2, ' '
        sb      t2, 0(t0)
.sign_done:
        addi    t0, t0, 1             # point to first digit position

        li      t3, 1000000000
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 100000000
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 10000000
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 1000000
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 100000
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 10000
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 1000
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 100
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        li      t3, 10
        divu    t4, t1, t3
        remu    t1, t1, t3
        addi    t4, t4, '0'
        sb      t4, 0(t0)
        addi    t0, t0, 1

        addi    t1, t1, '0'
        sb      t1, 0(t0)
        addi    t0, t0, 1
        sb      zero, 0(t0)
        ret
