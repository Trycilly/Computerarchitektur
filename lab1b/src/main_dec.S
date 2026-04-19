        .section .bss, "aw", @nobits
        .align  2

dec_buffer:
        .space  16                  # enough for sign + 10 digits + "\0"

        .text 
        .align  1
        .global main

main:
        addi    sp, sp, -8
        sw      ra, 4(sp)
        sw      s0, 0(sp)

        la      s0, dec_buffer      # persistent output buffer
        li      t0, -2147483648     # start at INT_MIN (corner case)

loop:   
        mv      a0, s0
        mv      a1, t0
        jal     decToASCII

        addi    t0, t0, 1

        j       loop

        # unreachable
        lw      s0, 0(sp)
        lw      ra, 4(sp)
        addi    sp, sp, 8
        ret
