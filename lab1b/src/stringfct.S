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

/**
 * Determines length of null-terminated string
 * a0 = address of string
 * returns a0 = length (without null terminator)
 */
my_strlen:
        mv      t0, a0                                  # t0 = current pointer
        li      t1, 0                                   # t1 = length counter

/** Loop: count characters until null terminator */
.len_loop:
        lbu     t2, 0(t0)                               # load current byte from memory
        beqz    t2, .len_done                           # if byte == 0 -> end of string
        addi    t1, t1, 1                               # increment length counter
        addi    t0, t0, 1                               # move pointer to next character 
        j       .len_loop

/** returns length */
.len_done:
        mv      a0, t1                                  # move length into return register 
        ret                                             # return to caller

.global my_strcpy

/**
 * Copies a null terminated string from source to destination
 * a0 = destination address
 * a1 = source address
 */
my_strcpy:
        mv      t0, a0                                  # t0 = destination pointer
        mv      t1, a1                                  # t1 = source pointer

/** Loop: copy byte-to-byte including null terminator */
.copy_loop:
        lbu     t2, 0(t1)                               # load byte from source 
        sb      t2, 0(t0)                               # store byte into destination
        beqz    t2, .copy_done                          # if byte == 0 -> end of string copied
        addi    t0, t0, 1                               # advance destination pointer 
        addi    t1, t1, 1                               # advance source pointer       
        j       .copy_loop              

/** Finished copying (including null terminator) */
.copy_done:
        ret                                             # return to caller

.global my_strtolower

/**
 * Converts all uppercase letters into lowercase 
 * a0 = address of string 
 * returns a0 = length of string
 */
my_strtolower:
        mv      t0, a0                                  # t0 = current pointer
        mv      t1, a0                                  # t1 = start pointer for length calculation
        li      t3, 'A'                                 # ASCII value of 'A'
        li      t4, 'Z'                                 # ASCII value of 'Z'

/** Loop: check each character and convert if needed */
.lower_loop:
        lbu     t2, 0(t0)                               # load current character 
        beqz    t2, .lower_done                         # if null terminator -> end of string
        blt     t2, t3, .lower_next                     # if < 'A' -> skip (not uppercase)
        bgt     t2, t4, .lower_next                     # if > 'Z' -> skip (not uppercase)
        ori     t2, t2, 0x20                            # ASCII uppercase -> lowercase (set ASCII bit 5)
        sb      t2, 0(t0)                               # store converted character back

.lower_next:
        addi    t0, t0, 1                               # move to next character
        j       .lower_loop

/** Compute and return string length */
.lower_done:
        sub     a0, t0, t1                              # length = end_ptr - start_ptr 
        ret                                             # return to caller 
