 # test main.S for hexToASCII.S
        .section const_section, "a"
        .align  2

        .section .bss, "aw", @nobits
        .align  2

hex_buffer:
        .space  12                      # enough for "0xFFFFFFFF\0"

        .text
        .align  1
        .global main

main:
        addi    sp, sp, -8
        sw      ra, 4(sp)
        sw      s0, 0(sp)

        la      s0, hex_buffer          # s0 = output buffer pointer
        li      t0, 0

loop:
        mv      a0, s0
        mv      a1, t0
        jal     hexToASCII

        addi    t0, t0, 1
        andi    t0, t0, 0xFFFF

        j       loop

        # unreachable
        lw      s0, 0(sp)
        lw      ra, 4(sp)
        addi    sp, sp, 8
        ret
