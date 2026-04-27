/** =========================
 * DATA / CONST (Keep this)
 * ========================= */
.section const_section, "a"
.align 2

test_string:
        .asciz "Test String ABC 0123!"

/** =========================
 * BSS (RAM BUFFERS)
 * ========================= */
.section .bss, "aw", @nobits
.align 2

ram_buffer:
        .space 64
hex_buffer:
        .space 12          # "0xFFFFFFFF\0"
dec_buffer:
        .space 16          # sign + digits + \0

/** =========================
 * MAIN TEXT SECTION
 * ========================= */
.section .text
.align 1
.global main

main:
        /** Prologue: Allocate stack space and save registers */
        addi    sp, sp, -32
        sw      ra, 28(sp)
        sw      s0, 24(sp)
        sw      s1, 20(sp)
        sw      s2, 16(sp)
        sw      s3, 12(sp)
        sw      s4, 8(sp)

        /** 1. STRING PIPELINE */
        la      a0, ram_buffer
        la      a1, test_string
        jal     my_strcpy

        la      a0, ram_buffer
        jal     my_strtolower

        /** 2. INIT POINTERS & COUNTERS */
        la      s1, hex_buffer
        la      s2, dec_buffer
        
        li      s3, 0               # s3 = hex counter (Saved register)
        li      s4, -10             # s4 = dec counter (Saved register)

/** 3. MAIN LOOP */
loop:
        /** ---- HEX PART ---- */
        mv      a0, s1              # Buffer
        mv      a1, s3              # Value
        jal     hexToASCII

        # PRINT HEX
        mv      a0, s1
        li      a7, 4               # Syscall: Print String
        ecall

        # PRINT SPACE
        li      a0, ' '
        li      a7, 11              # Syscall: Print Char
        ecall

        /** ---- DEC PART ---- */
        mv      a0, s2              # Buffer
        mv      a1, s4              # Value
        jal     decToASCII

        # PRINT DEC
        mv      a0, s2
        li      a7, 4               # Syscall: Print String
        ecall

        # PRINT NEWLINE
        li      a0, '\n'
        li      a7, 11              # Syscall: Print Char
        ecall

        /** ---- UPDATE COUNTERS ---- */
        addi    s3, s3, 1           # Increment Hex
        li      t2, 0xFFFF
        and     s3, s3, t2          # Mask to 16-bit

        addi    s4, s4, 1           # Increment Dec

        j       loop                # Infinite Loop

/** EPILOG (Safeguard) */
        lw      s4, 8(sp)
        lw      s3, 12(sp)
        lw      s2, 16(sp)
        lw      s1, 20(sp)
        lw      s0, 24(sp)
        lw      ra, 28(sp)
        addi    sp, sp, 32
        ret
