/** =========================
 * TEXT
 * ========================= */
.section .text
.align 1
.global main

main:
        /** Prologue: Saving RA and S registers to the stack */
        addi    sp, sp, -32
        sw      ra, 28(sp)
        sw      s0, 24(sp)
        sw      s1, 20(sp)
        sw      s2, 16(sp)
        sw      s3, 12(sp)
        sw      s4, 8(sp)

        /** ---- STRING PIPELINE ---- */
        la      a0, test_string
        jal     my_strcpy              # Copy to RAM buffer
        
        la      a0, ram_buffer
        jal     my_strtolower          # Convert to lowercase

        /** ---- INIT POINTERS & COUNTERS ---- */
        la      s1, hex_buffer         # Pointer to hex buffer
        la      s2, dec_buffer         # Pointer to dec buffer
        
        li      s3, 0                  # s3 = hex counter (replaces t0)
        li      s4, -10                # s4 = dec counter (replaces t1) 
                                       # Note: Starting at -10 for testing readability

/** =========================
 * MAIN LOOP
 * ========================= */
loop:
        /** ---- 1. PROCESS & PRINT HEX ---- */
        mv      a0, s1                 # Destination buffer
        mv      a1, s3                 # Value to convert
        jal     hexToASCII

        mv      a0, s1                 # Load buffer address to print
        li      a7, 4                  # Syscall 4: Print String
        ecall                          # Execute Print

        # Print a space for formatting
        li      a0, ' '
        li      a7, 11                 # Syscall 11: Print Character
        ecall

        /** ---- 2. PROCESS & PRINT DEC ---- */
        mv      a0, s2                 # Destination buffer
        mv      a1, s4                 # Value to convert
        jal     decToASCII

        mv      a0, s2                 # Load buffer address to print
        li      a7, 4                  # Syscall 4: Print String
        ecall

        # Print a newline for formatting
        li      a0, '\n'
        li      a7, 11                 # Syscall 11: Print Character
        ecall

        /** ---- 3. UPDATE COUNTERS ---- */
        addi    s3, s3, 1              # Increment hex counter
        li      t2, 0xFFFF
        and     s3, s3, t2             # Keep it within 16-bit range

        addi    s4, s4, 1              # Increment dec counter

        # Optional: Add a break condition or small loop limit here
        # For now, this will loop continuously as requested
        j       loop

/** =========================
 * EPILOG (If loop ever exits)
 * ========================= */
        lw      s4, 8(sp)
        lw      s3, 12(sp)
        lw      s2, 16(sp)
        lw      s1, 20(sp)
        lw      s0, 24(sp)
        lw      ra, 28(sp)
        addi    sp, sp, 32
        ret
