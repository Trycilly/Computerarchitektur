/** Deprecated */

.section const_section, "a"                        # Read-only constants
        .align  2
        .global  .test_string
.test_string:
        .asciz  "Test String ABC 0123!"

        .section .bss, "aw", @nobits                      # RAM storage for conversion results
        .align  2
        .global  hex_buffer
hex_buffer:
        .space  12
        .align  2
        .global  dec_buffer
dec_buffer:
        .space  16

        .text                                              # Mark the next section as .text/.code section
        .align  1                                          # Aligned to 2^1 Bytes (aka 2 Bytes)
        .global  main                                      # Defines a globally-visible (exported) symbol "main", visible to the linker ld
main:                                                      # The location of the Symbol "main": here, the function starts
        # 0. Save the callee-saved registers ra, s1, s2 onto the stack
        addi    sp, sp, -12                                # Allocate data on the stack: 12 Bytes
        sw      ra, 8(sp)                                  # Store the return address
        sw      s1, 4(sp)                                  # Save s1
        sw      s2, 0(sp)                                  # Save s2

        la      s1, hex_buffer                             # pointer to hex output buffer
        la      s2, dec_buffer                             # pointer to decimal output buffer
        li      t0, 0                                      # 16-bit test counter for hex conversion
        li      t1, 0                                      # 32-bit signed test counter for decimal conversion

.main_loop:
        mv      a0, s1
        mv      a1, t0
        jal     hexToASCII

        mv      a0, s2
        mv      a1, t1
        jal     decToASCII

        addi    t0, t0, 1
        andi    t0, t0, 0xffff
        addi    t1, t1, 1
        j       .main_loop                                 # Repeat forever

        # ####################################### #
        # Unreachable cleanup - kept for best practice
        lw      s2, 0(sp)                                  # Restore s2
        lw      s1, 4(sp)                                  # Restore s1
        lw      ra, 8(sp)                                  # Restore ra
        addi    sp, sp, 12                                 # Restore stack
        ret                                                # Return to caller
