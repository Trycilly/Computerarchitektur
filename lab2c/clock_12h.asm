; HCS12 Clock Display with Conditional 12h/24h Mode
; Conditional assembly using SELECT12HOURS symbol
; Set SELECT12HOURS to 0 for 24h mode, 1 for 12h mode

SELECT12HOURS: EQU 1  ; 0 = 24h mode, 1 = 12h mode

; Memory locations for time storage
ORG $2000
hours:    DS.B 1      ; Current hour (0-23 for 24h, 0-23 for 12h calculation)
minutes:  DS.B 1      ; Current minute (0-59)
seconds:  DS.B 1      ; Current second (0-59)

IF SELECT12HOURS == 1
am_pm:    DS.B 1      ; 0 = AM, 1 = PM (only used in 12h mode)
ENDIF

; Display buffer for formatted time string
display_buffer: DS.B 12   ; Format: "HH:MM:SS" or "HH:MM:SS AM/PM"

; ============================================================================
; Main Clock Update Routine
; Called to increment time by 1 second
; ============================================================================
ORG $3000
clock_inc_second:
    ; Increment seconds
    LDX #seconds
    INC 0,X
    LDAB #60
    CMPB 0,X
    BHI clock_inc_end      ; if seconds < 60, done
    
    ; Seconds overflow, reset and increment minutes
    CLRB
    STAB 0,X
    
    LDX #minutes
    INC 0,X
    LDAB #60
    CMPB 0,X
    BHI clock_inc_end      ; if minutes < 60, done
    
    ; Minutes overflow, reset and increment hours
    CLRB
    STAB 0,X
    
    LDX #hours
    INC 0,X

IF SELECT12HOURS == 0
    ; 24h mode: wrap at 24
    LDAB #24
ELSE
    ; 12h mode: wrap at 12 for display, but track 0-23 internally
    ; This is handled in the format routine
    LDAB #24
ENDIF
    
    CMPB 0,X
    BHI clock_inc_end
    
    ; Hour overflow, reset to 0
    CLRB
    STAB 0,X

clock_inc_end:
    RTS

; ============================================================================
; Format Time for Display
; Formats the current time and prepares display string
; ============================================================================
ORG $3050
clock_format_display:
    ; Load current time values
    LDX #hours
    LDAB 0,X         ; B = hours
    
    LDX #minutes
    LDAA 0,X         ; A = minutes
    PSHA              ; Push minutes for later use
    
    LDX #seconds
    LDAC 0,X         ; C = seconds
    PSHA              ; Push seconds for later use

IF SELECT12HOURS == 1
    ; 12h mode conversion
    ; Check if we should display PM
    CMPB #12
    BLO am_mode       ; if hours < 12, it's AM
    
    ; Hours >= 12, PM time
    MOVB #1, am_pm
    
    ; Convert to 12h display (13->1, 14->2, etc.)
    CMPB #12
    BEQ pm_12        ; if hours == 12, display as 12 (noon case)
    SUBB #12         ; hours > 12, subtract 12
    BRA format_convert
    
pm_12:
    ; Hours = 12 (noon), display as 12
    BRA format_convert
    
am_mode:
    ; Hours < 12, AM time
    MOVB #0, am_pm
    
    ; Convert 0 -> 12, 1-11 stay same
    BNE format_convert  ; if hours != 0, don't change
    MOVB #12, 0,X       ; midnight: display as 12
    
format_convert:
    ; B now contains the display hour (1-12)
ELSE
    ; 24h mode: B already contains hours (0-23)
ENDIF

    ; Format and output the string
    ; This would interface with your display system
    ; For now, we'll just prepare the values
    
    PULA              ; A = seconds
    PULB              ; B = minutes (were pushed in reverse order)
    
    RTS

; ============================================================================
; Time Overflow Test Routine - Validates 12h transitions
; ============================================================================
ORG $3100
test_12h_transitions:

IF SELECT12HOURS == 1

    ; Test 11:59:59 AM -> 12:00:00 PM
    MOVB #11, hours
    MOVB #59, minutes
    MOVB #59, seconds
    MOVB #0, am_pm        ; AM
    
    JSR clock_inc_second  ; Should roll to 12:00:00 PM
    
    ; Verify: hours should be 12, minutes 0, seconds 0, am_pm should be 1 (PM)
    LDX #hours
    LDAB 0,X
    CMPB #12
    BNE test_fail
    
    LDX #minutes
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #seconds
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #am_pm
    LDAB 0,X
    CMPB #1             ; Should be PM
    BNE test_fail
    
    ; Test 12:59:59 PM -> 01:00:00 PM
    MOVB #12, hours
    MOVB #59, minutes
    MOVB #59, seconds
    MOVB #1, am_pm        ; PM
    
    JSR clock_inc_second  ; Should roll to 13:00:00 (1 PM in 24h format)
    
    ; Verify: hours should be 13, minutes 0, seconds 0
    LDX #hours
    LDAB 0,X
    CMPB #13
    BNE test_fail
    
    LDX #minutes
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #seconds
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    ; Test 11:59:59 PM -> 12:00:00 AM
    MOVB #23, hours
    MOVB #59, minutes
    MOVB #59, seconds
    MOVB #1, am_pm        ; PM
    
    JSR clock_inc_second  ; Should roll to 00:00:00 (12 AM)
    
    ; Verify: hours should be 0, minutes 0, seconds 0, am_pm should be 0 (AM)
    LDX #hours
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #minutes
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #seconds
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #am_pm
    LDAB 0,X
    CMPB #0             ; Should be AM
    BNE test_fail
    
    ; Test 12:59:59 AM -> 01:00:00 AM
    MOVB #0, hours      ; 12 AM in 24h format
    MOVB #59, minutes
    MOVB #59, seconds
    MOVB #0, am_pm        ; AM
    
    JSR clock_inc_second  ; Should roll to 01:00:00 (1 AM)
    
    ; Verify: hours should be 1, minutes 0, seconds 0, am_pm should be 0 (AM)
    LDX #hours
    LDAB 0,X
    CMPB #1
    BNE test_fail
    
    LDX #minutes
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #seconds
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #am_pm
    LDAB 0,X
    CMPB #0             ; Should be AM
    BNE test_fail
    
    ; All tests passed
    MOVB #1, test_result
    BRA test_end

ELSE

    ; 24h mode tests
    ; Test 23:59:59 -> 00:00:00
    MOVB #23, hours
    MOVB #59, minutes
    MOVB #59, seconds
    
    JSR clock_inc_second
    
    LDX #hours
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #minutes
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    LDX #seconds
    LDAB 0,X
    CMPB #0
    BNE test_fail
    
    MOVB #1, test_result
    BRA test_end

ENDIF

test_fail:
    MOVB #0, test_result
    
test_end:
    RTS

ORG $3200
test_result: DS.B 1   ; 1 = pass, 0 = fail

END
