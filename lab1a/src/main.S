        .section const_section, "a"                        # Here comes a true read-only (const) data section for LED_PINS
        .align  2                                          # Aligned to 2^2 Bytes (aka 4 Bytes)
        .global .LED_PIN
        .type   .LED_PIN, @object                          # Mark the symbol as being a data object
.LED_PIN:                                                  # Define the array with the LED's GPIO pins
        .word   5
.equ    LED_PIN_BYTES, (. -.LED_PIN)                       # Constant! GAS trick: "." stores the current address (aka offset from beginning) in Bytes

        .text                                              # Mark the next section as .text/.code section
        .align  1                                          # Aligned to 2^1 Bytes (aka 2 Bytes)
        .global  main                                      # Defines a globally-visible (exported) symbol "main", visible to the linker ld
main:                                                      # The location of the Symbol "main": here, the function starts
        # 0. Safe the used callee-saved registers ra, s0-s2 onto the stack
        addi    sp, sp, -12                                # Allocate data on the stack
        sw      ra, 0(sp)                                  # Store the return address (ra) register on the stack
        sw      s1, 4(sp)                                  # Store the saved register s1 on the stack
        sw      s2, 8(sp)                                  # Store the saved register s2 on the stack

        # 1. Initialization: Use s1 as pointer to LED_PINS, while s2 is the end-pointer (both unchanged below)
        lui     s1, %hi(.LED_PIN)                          # Initialize with Pointer to first element by loading upper 20-bit
        addi    s1, s1, %lo(.LED_PIN)                      # and adding the lower 12 bit with GAS macros %hi() and %lo()
        lw      a0, 0(s1)                                  # Load the word at s1[0], aka dereference s1 into a0
        jal     led_init                                   # Jump-and-link to function led_init

        # 2. Setup the loop state
        li      t2, 5                                      # LED GPIO pin number
        li      t6, 0                                      # 16-bit counter starting at 0

        # 3. Loop forever and blink the LED every other count
        .main_loop:                                                
                andi    t1, t6, 1           # Check odd/even: t1=0 (Even), t1=1 (Odd)
                beqz    t1, .led_on_branch  
                
                mv      a0, t2              # Set a0 = 5
                jal     led_off             
                j       .after_led          # Skip the ON section

.led_on_branch:
        # If we are here, it's EVEN -> Turn LED ON
        mv      a0, t2              # Set a0 = 5
        jal     led_on              
        
.after_led:
        jal     my_delay        # Wait 0.5 seconds
        addi    t6, t6, 1           # Increment counter
        li      t3, 64              # Load our limit into a spare register
        blt     t6, t3, .main_loop  # If t6 < 64, loop again. 
                                    # If t6 == 64, it "falls through" to exit.

        # ####################################### #
        # In fact, the following clean-up code will never be executed... Still keep it, to show best practice
        lw      s2, 8(sp)                                  # Restore the saved register s2 from the stack
        lw      s1, 4(sp)                                  # Restore the saved register s1 from the stack
        lw      ra, 0(sp)                                  # Restore the saved register ra from the stack
        addi    sp, sp, 12                                 # Clean up the stack-pointer
        ret                                                # Return to the caller
        
