/**
 * \file stringfct.S
 * \brief String manipulation routines for lab1b.
 *
 * This file implements three assembler routines:
 *  - my_strlen: returns length of NUL-terminated ASCII string
 *  - my_strcpy: copies an ASCIIZ string from source to destination
 *  - my_strtolower: converts upper-case ASCII letters to lower-case in-place
 */

        .text
        .align 1

        .global my_strlen
my_strlen:
        mv      t0, a0              # t0 = current pointer
        li      t1, 0               # t1 = length counter
.len_loop:
        lbu     t2, 0(t0)           # load next byte
        beqz    t2, .len_done       # end of string?
        addi    t1, t1, 1
        addi    t0, t0, 1
        j       .len_loop
.len_done:
        mv      a0, t1
        ret

        .global my_strcpy
my_strcpy:
        mv      t0, a0              # t0 = destination pointer
        mv      t1, a1              # t1 = source pointer
.copy_loop:
        lbu     t2, 0(t1)
        sb      t2, 0(t0)
        beqz    t2, .copy_done
        addi    t0, t0, 1
        addi    t1, t1, 1
        j       .copy_loop
.copy_done:
        ret

        .global my_strtolower
my_strtolower:
        mv      t0, a0              # t0 = current pointer
        mv      t1, a0              # t1 = start pointer for length calculation
.lower_loop:
        lbu     t2, 0(t0)
        beqz    t2, .lower_done
        li      t3, 'A'
        li      t4, 'Z'
        blt     t2, t3, .lower_next
        bgt     t2, t4, .lower_next
        ori     t2, t2, 0x20        # ASCII uppercase -> lowercase
        sb      t2, 0(t0)
.lower_next:
        addi    t0, t0, 1
        j       .lower_loop
.lower_done:
        sub     a0, t0, t1          # return string length in a0
        ret
