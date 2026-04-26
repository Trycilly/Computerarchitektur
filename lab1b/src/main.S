/** =========================
 *  DATA / CONST
 *  ========================= */
.section const_section, "a"
.align 2

test_string:
        .asciz "Test String ABC 0123!"


/** =========================
 *  BSS (RAM BUFFERS)
 *  ========================= */
.section .bss, "aw", @nobits
.align 2

ram_buffer:
        .space 64

hex_buffer:
        .space 12          # "0xFFFFFFFF\0"

dec_buffer:
        .space 16          # sign + digits + \0


/** =========================
 *  TEXT
 *  ========================= */
.section .text
.align 1
.global main


main:
        /** Prologue */
        addi    sp, sp, -16
        sw      ra, 12(sp)
        sw      s0, 8(sp)
        sw      s1, 4(sp)
        sw      s2, 0(sp)

        /** =========================
         *  STRING PIPELINE (einmalig)
         *  ========================= */

        /* strlen */
        la      a0, test_string
        jal     my_strlen
        mv      s0, a0                 # Länge (optional gespeichert)

        /* strcpy */
        la      a0, ram_buffer
        la      a1, test_string
        jal     my_strcpy

        /* strtolower */
        la      a0, ram_buffer
        jal     my_strtolower


        /** =========================
         *  INIT POINTERS
         *  ========================= */

        la      s1, hex_buffer         # hex buffer
        la      s2, dec_buffer         # dec buffer

        li      t0, 0                  # hex counter
        li      t1, -2147483648        # dec counter


/** =========================
 *  MAIN LOOP (HEX + DEC parallel)
 *  ========================= */
loop:

        /** ---- HEX ---- */
        mv      a0, s1
        mv      a1, t0
        jal     hexToASCII

        addi    t0, t0, 1
        andi    t0, t0, 0xFFFF


        /** ---- DEC ---- */
        mv      a0, s2
        mv      a1, t1
        jal     decToASCII

        addi    t1, t1, 1


        j       loop


/** =========================
 *  EPILOG (nie erreicht)
 *  ========================= */
        lw      s2, 0(sp)
        lw      s1, 4(sp)
        lw      s0, 8(sp)
        lw      ra, 12(sp)
        addi    sp, sp, 16
        ret
