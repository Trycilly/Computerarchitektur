# 12-Hour Clock Mode Implementation

## Overview

This document describes the implementation of 12-hour and 24-hour clock display modes using conditional compilation. The project provides two versions:

1. **HCS12 Assembler Version** (`clock_12h.asm`)
2. **C Language Version** (C source files with conditional compilation)

## Conditional Compilation Symbol

### Symbol Definition: `SELECT_12HOURS`

Set this symbol to control which mode is compiled:

- **`SELECT_12HOURS = 0`**: 24-hour mode (00:00 - 23:59)
- **`SELECT_12HOURS = 1`**: 12-hour mode with AM/PM indicator

### Location

**C Version:** Defined in `src/clock_time.h`
```c
#define SELECT_12HOURS 1  /* Set to 0 for 24h mode, 1 for 12h mode */
```

**HCS12 Assembler:** Defined in `clock_12h.asm`
```asm
SELECT12HOURS: EQU 1  ; 0 = 24h mode, 1 = 12h mode
```

## 12-Hour Time System Rules

The following table shows how hours are displayed and when AM/PM indicators are used:

### Time Conversion Table

| 24-hour | Display (12h) | Indicator | Description |
|---------|---------------|-----------|-------------|
| 00:00   | 12:00         | AM        | Midnight |
| 01:00   | 01:00         | AM        | 1 AM |
| 02:00   | 02:00         | AM        | 2 AM |
| ...     | ...           | AM        | ... |
| 11:00   | 11:00         | AM        | 11 AM |
| 12:00   | 12:00         | PM        | Noon |
| 13:00   | 01:00         | PM        | 1 PM |
| 14:00   | 02:00         | PM        | 2 PM |
| ...     | ...           | PM        | ... |
| 23:00   | 11:00         | PM        | 11 PM |

## Overflow Transitions

The critical transitions when seconds, minutes, and hours overflow are handled correctly:

### Transition 1: 11:59:59 AM → 12:00:00 PM
```
24h format: 11:59:59 → 12:00:00
12h display: 11:59:59 AM → 12:00:00 PM
```
**Behavior:** Seconds roll over, triggering minute and hour rollovers. At 12:00, the display transitions from AM to PM.

### Transition 2: 12:59:59 PM → 01:00:00 PM
```
24h format: 12:59:59 → 13:00:00
12h display: 12:59:59 PM → 01:00:00 PM
```
**Behavior:** The hour changes from 12 to 1 (13 in 24h), while remaining in PM.

### Transition 3: 11:59:59 PM → 12:00:00 AM
```
24h format: 23:59:59 → 00:00:00
12h display: 11:59:59 PM → 12:00:00 AM
```
**Behavior:** Midnight transition—all values roll over (hours from 23 to 0), and indicator changes from PM to AM.

### Transition 4: 12:59:59 AM → 01:00:00 AM
```
24h format: 00:59:59 → 01:00:00
12h display: 12:59:59 AM → 01:00:00 AM
```
**Behavior:** Early morning transition—hour changes from 0 to 1 (from 12 AM to 1 AM), remaining in AM.

## Implementation Details

### C Language Implementation

#### 1. Core Time Functions (Modified)

**File:** `src/clock_time.c`

The `clock_time_inc_second()` function handles cascade overflow:
```c
// Cascade logic ensures proper overflow handling
seconds++;
if (seconds >= 60) {
    seconds = 0;
    minutes++;
    if (minutes >= 60) {
        minutes = 0;
        hours++;
        if (hours >= 24) {
            hours = 0;
        }
    }
}
```

#### 2. Format String Function (Conditional)

```c
void clock_time_format_string(char *buffer, int hour, int minute, int second, bool use_12h) {
#if SELECT_12HOURS == 1
    if (use_12h) {
        int hour_12 = hour % 12;
        if (hour_12 == 0) hour_12 = 12;  // Convert 0 to 12
        const char *am_pm = (hour >= 12) ? "PM" : "AM";
        sprintf(buffer, "%02d:%02d:%02d %s", hour_12, minute, second, am_pm);
    } else {
        sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
    }
#else
    sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
#endif
}
```

#### 3. New Helper Function

**File:** `src/clock_time.h` and `clock_time.c`

```c
#if SELECT_12HOURS == 1
bool clock_time_is_pm(void);
#endif
```

This function returns `true` if the current time is in PM (12:00-23:59).

#### 4. Main Loop Adaptation (Modified)

**File:** `src/clock.c`

The button handling is now conditional on the compilation mode:

```c
#if SELECT_12HOURS == 1
static bool use_12h = true;   /* Start in 12h mode */
#else
static bool use_12h = false;  /* 24h mode only */
#endif
```

When `SELECT_12HOURS == 0`, the mode button is disabled.

### HCS12 Assembler Implementation

**File:** `clock_12h.asm`

The assembler version uses HCS12 conditional assembly directives:

#### Clock Increment Routine
```asm
clock_inc_second:
    LDX #seconds
    INC 0,X
    LDAB #60
    CMPB 0,X
    BHI clock_inc_end      ; if seconds < 60, done
    
    CLRB
    STAB 0,X
    
    ; Continue with minutes and hours cascade...
```

#### Hour Overflow (Mode-Dependent)
```asm
IF SELECT12HOURS == 0
    ; 24h mode: wrap at 24
    LDAB #24
ELSE
    ; 12h mode: wrap at 24 (internal representation)
    LDAB #24
ENDIF
```

#### 12h Test Suite
```asm
test_12h_transitions:
IF SELECT12HOURS == 1
    ; Test 11:59:59 AM -> 12:00:00 PM
    MOVB #11, hours
    MOVB #59, minutes
    MOVB #59, seconds
    JSR clock_inc_second
    
    ; Verify results...
ENDIF
```

## Testing

A comprehensive test suite is provided in `src/clock_time_test.c`.

### Running Tests

Call `clock_time_run_tests()` from your main function or test framework.

### Test Cases

1. **Noon Transition:** 11:59:59 → 12:00:00 (AM to PM)
2. **Afternoon Transition:** 12:59:59 PM → 01:00:00 PM
3. **Midnight Transition:** 23:59:59 → 00:00:00 (PM to AM)
4. **Early Morning:** 00:59:59 → 01:00:00 (stays in AM)
5. **Minute Overflow Cascade:** Multiple second increments
6. **Hour Overflow Cascade:** Day wraparound
7. **Format String Verification:** Correct AM/PM and hour display

### Expected Test Output

```
╔════════════════════════════════════════════════════════════════╗
║        HCS12 12-Hour Clock Overflow Transition Tests          ║
╚════════════════════════════════════════════════════════════════╝

[Mode: 12-HOUR DISPLAY]

--- Test 1: Noon Transition (11:59:59 → 12:00:00) ---
Setup: 11:59:59
✓ PASS: Noon transition - 12:00:00

--- Test 2: Afternoon Transition (12:59:59 PM → 01:00:00 PM) ---
Setup: 12:59:59
✓ PASS: Afternoon transition (13:00:00 in 24h) - 13:00:00

... [More tests] ...

╔════════════════════════════════════════════════════════════════╗
║                        TEST SUMMARY                           ║
╠════════════════════════════════════════════════════════════════╣
║  Tests Passed:   7                                            ║
║  Tests Failed:   0                                            ║
║  Total Tests:    7                                            ║
╚════════════════════════════════════════════════════════════════╝

✓ ALL TESTS PASSED!
```

## Configuration Instructions

### For C Project

1. **Edit** `src/clock_time.h`
2. **Modify** the line:
   ```c
   #define SELECT_12HOURS 1  /* Change to 0 for 24h mode */
   ```
3. **Rebuild** your project
4. The GUI will automatically use the configured mode

### For HCS12 Assembler Project

1. **Edit** `clock_12h.asm`
2. **Modify** the first line:
   ```asm
   SELECT12HOURS: EQU 1  ; Change to 0 for 24h mode
   ```
3. **Assemble** the file with your HCS12 assembler (CodeWarrior)
4. The assembler will conditionally include the appropriate code sections

## Conditional Compilation Directives

### C Language
```c
#if SELECT_12HOURS == 1
    /* 12h mode specific code */
#else
    /* 24h mode specific code */
#endif
```

### HCS12 Assembler
```asm
IF SELECT12HOURS == 1
    ; 12h mode specific code
ELSE
    ; 24h mode specific code
ENDIF
```

## Time Display Format

### 24-Hour Mode
```
HH:MM:SS
00:00:00  (midnight)
12:30:45  (afternoon)
23:59:59  (late night)
```

### 12-Hour Mode
```
HH:MM:SS AM/PM
12:00:00 AM  (midnight)
12:30:45 PM  (afternoon)
11:59:59 PM  (late night)
```

## Implementation Notes

1. **Internal Representation:** Internally, time is always stored in 24-hour format (0-23 hours). The 12-hour conversion happens only during display formatting.

2. **UTC vs Local Time:** The implementation respects timezone offsets. The `clock_time_get_local()` function applies timezone adjustments before conversion.

3. **Memory Efficiency (Assembler):** In the HCS12 version, the `am_pm` byte is conditionally allocated only when `SELECT12HOURS == 1`.

4. **Thread Safety:** If used in a multithreaded environment, consider adding mutex protection around the time access functions.

## Compatibility

- **C Version:** Compatible with any C99 or later compiler
- **HCS12 Version:** Compatible with CodeWarrior HCS12 assembler
- **Testing:** Unit tests use standard C, compatible with any test framework

## References

- 12-hour clock: https://en.wikipedia.org/wiki/12-hour_clock
- HCS12 Conditional Assembly: CodeWarrior Documentation → Assembler → Conditional Assembly Directives
- C Preprocessing: C11/C99 Standard → Preprocessor Directives

## License

Provided as-is for educational purposes.
