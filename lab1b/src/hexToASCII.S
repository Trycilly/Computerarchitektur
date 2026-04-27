/**
 * \file hexToASCII.S
 * \brief Convert a 32-bit value to a NUL-terminated ASCII hex string.
 *
 * The caller passes the destination pointer in a0 and the value in a1.
 * The result is always formatted as "0x1234ABCD" with exactly 8 hex digits.
 */

        .section const_section, "a"                     # constant (read only) section
        .align  2                                       # 4 byte Alignment 
        .global H2A                                     # Lookup Table globally visible

/** Lookup Table for hexadecimal digits (0–F) */
H2A:
        .asciz  "0123456789ABCDEF"                      # Lookup Table, null-terminated with Hex characters

        .text
        .align 1                                        # 2 Byte Alignment for instructions
        .global hexToASCII

/**
 * Converts a 32 bit value into a null-terminated hex string
 * Format: "0xXXXXXXXX"
 * a0 = destination buffer
 * a1 = value to convert
 */
hexToASCII:
        mv      t0, a0                                  # t0 = pointer to destination buffer 
        li      t1, '0'                                 # load ASCII '0'
        sb      t1, 0(t0)                               # store '0' at buffer[0]
        li      t1, 'x'                                 # load ASCII 'x'
        sb      t1, 1(t0)                               # store 'x' at buffer[1]
        addi    t0, t0, 2                               # advance pointer past "0x"
        mv      t2, a1                                  # t2 = value to convert
        li      t3, 28                                  # t3 = bit shift (start at highest nibble: 28)
        li      t4, 0xf                                 # t4 = mask for one hex digit (4 bits)
        la      t6, H2A                                 # load address of lookup table

/** Loop: extract each 4-bit nibble and map it to ASCII */
.hex_loop:
        srl     t5, t2, t3                              # shift desired nibble to lowest 4 bits
        and     t5, t5, t4                              # isolate the lowest 4 bits (0-15) 
        add     t5, t6, t5                              # index into table using nibble value
        lbu     t5, 0(t5)                               # load corresponding ASCII character
        sb      t5, 0(t0)                               # store character in destination buffer
        addi    t0, t0, 1                               # advance destination pointer
        addi    t3, t3, -4                              # move to next nibble (shift right by 4 bits)
        bgez    t3, .hex_loop                           # repeat until all 8 nibbles processed

        /** Nulltermination of string */
        sb      zero, 0(t0)                             # append '\0' at the end of the string
        ret                                             # return to caller 
