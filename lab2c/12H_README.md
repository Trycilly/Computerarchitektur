# 12-Hour Clock Mode Configuration Guide

## Quick Start

### For C/Pico Project Users

1. **Switch Between 12h and 24h Mode:**
   - Edit `src/clock_time.h` (line 11-14)
   - Change:
     ```c
     #define SELECT_12HOURS 1  /* 1 = 12h mode, 0 = 24h mode */
     ```
   - Rebuild your project

2. **Hardware Button:**
   - GPIO pin 2: 12h/24h mode toggle button
   - Connected to GND with pull-up resistor
   - Press to switch between modes (only active in 12h mode config)

3. **Testing:**
   - Call `clock_time_run_tests()` from your code to run comprehensive overflow tests
   - Tests verify all critical 12h transitions

---

### For HCS12 Assembler Users

1. **Open** `clock_12h.asm`

2. **Set Mode:**
   ```asm
   SELECT12HOURS: EQU 1  ; Set to 0 for 24h, 1 for 12h
   ```

3. **Key Routines:**
   - `clock_inc_second` - Increments time with overflow cascade
   - `clock_format_display` - Converts time to 12h format
   - `test_12h_transitions` - Validates all critical transitions

4. **Assemble** with CodeWarrior HCS12 assembler

---

## 12-Hour Mode Behavior

### Display Format
```
12h Mode:    12:00:00 AM  →  12:00:00 PM  →  11:59:59 PM  →  12:00:00 AM
24h Mode:    00:00:00    →  12:00:00    →  23:59:59    →  00:00:00
```

### Critical Transitions (Verified by Tests)

| Transition | 24h Format | 12h Display | Notes |
|-----------|-----------|-------------|-------|
| Midnight  | 11:59:59 → 12:00:00 | 11:59:59 AM → 12:00:00 PM | Noon marker |
| Early PM  | 12:59:59 → 13:00:00 | 12:59:59 PM → 01:00:00 PM | Hour resets to 01 |
| Late PM   | 23:59:59 → 00:00:00 | 11:59:59 PM → 12:00:00 AM | Midnight marker, day wrap |
| Early AM  | 00:59:59 → 01:00:00 | 12:59:59 AM → 01:00:00 AM | Hour increments from 12 |

---

## File Structure

```
lab2c/
├── CMakeLists.txt
├── 12H_MODE_DOCUMENTATION.md         ← Detailed technical docs
├── 12H_README.md                      ← This file
├── clock_12h.asm                      ← HCS12 assembly version
├── src/
│   ├── clock_time.h                   ← Conditional compilation flags
│   ├── clock_time.c                   ← Time functions (modified)
│   ├── clock_time_test.c              ← Unit tests (NEW)
│   ├── clock.c                        ← Main loop (modified)
│   ├── clock_gui.c                    ← Display handling
│   ├── clock_cursor.c                 ← Cursor/edit functionality
│   └── CMakeLists.txt                 ← Build config (updated)
└── images/
    └── watch_man.h                    ← Clock face background image
```

---

## Compilation Modes

### Mode 1: 24-Hour Display (Default)
```c
#define SELECT_12HOURS 0
```
- Time displays as `HH:MM:SS` (00:00-23:59)
- No AM/PM indicator
- Mode button disabled
- Slightly smaller code size

### Mode 2: 12-Hour Display
```c
#define SELECT_12HOURS 1
```
- Time displays as `HH:MM:SS AM/PM`
- Mode button (GPIO2) toggles 12h/24h display
- Includes 12h conversion logic
- All overflow tests validate 12h transitions

---

## Testing Your Implementation

### C Project Testing

Add this to your `main()` or create a test function:

```c
#include "clock_time.h"

// In main():
clock_time_run_tests();  // Prints test results to serial console
```

**Output Example:**
```
╔════════════════════════════════════════════════════════════════╗
║        HCS12 12-Hour Clock Overflow Transition Tests          ║
╚════════════════════════════════════════════════════════════════╝

[Mode: 12-HOUR DISPLAY]

--- Test 1: Noon Transition (11:59:59 → 12:00:00) ---
✓ PASS: Noon transition - 12:00:00

--- Test 2: Afternoon Transition (12:59:59 PM → 01:00:00 PM) ---
✓ PASS: Afternoon transition (13:00:00 in 24h) - 13:00:00

--- Test 3: Midnight Transition (11:59:59 PM → 12:00:00 AM) ---
✓ PASS: Midnight transition - 00:00:00

--- Test 4: Early Morning Transition (12:59:59 AM → 01:00:00 AM) ---
✓ PASS: Early morning transition - 01:00:00

--- Test 5: Minute Overflow ---
✓ PASS: Minute overflow (11:59:30 + 30s → 12:00:00) - 12:00:00

--- Test 6: Hour Overflow ---
✓ PASS: Hour overflow (23:30:00 + 30min → 00:00:00) - 00:00:00

--- Test 7: 12h Format Strings ---
✓ PASS: Midnight formatted correctly
✓ PASS: Noon formatted correctly
✓ PASS: Afternoon formatted correctly

╔════════════════════════════════════════════════════════════════╗
║                        TEST SUMMARY                           ║
╠════════════════════════════════════════════════════════════════╣
║  Tests Passed:   7                                            ║
║  Tests Failed:   0                                            ║
║  Total Tests:    7                                            ║
╚════════════════════════════════════════════════════════════════╝

✓ ALL TESTS PASSED!
```

### Manual Testing

1. **Set time to 11:59:59 AM** (internal: 11:59:59)
   ```c
   clock_time_set_utc(11, 59, 59);
   ```

2. **Increment once** (simulating 1 second passing)
   ```c
   clock_time_inc_second(time_us_64());
   ```

3. **Verify display shows 12:00:00 PM**
   - Expected: Hour display changes to 12
   - Expected: AM/PM indicator changes to PM

4. **Repeat for other critical transitions**

---

## Implementation Highlights

### Cascade Logic (Time Overflow)
The implementation uses cascading overflow checking:
```c
seconds++
if (seconds >= 60) {
    seconds = 0
    minutes++
    if (minutes >= 60) {
        minutes = 0
        hours++
        if (hours >= 24) {
            hours = 0  // Day wraparound
        }
    }
}
```

This ensures all overflow scenarios are correctly handled:
- ✓ 59:59 + 1s → 00:00 (seconds wrap)
- ✓ 59 minutes + 1m → hour increment
- ✓ 23:59:59 + 1s → 00:00:00 (day wrap)

### 12h Display Conversion
```c
int hour_12 = hour % 12;        // 13 % 12 = 1
if (hour_12 == 0) hour_12 = 12; // 0 becomes 12
const char *am_pm = (hour >= 12) ? "PM" : "AM";
```

This correctly maps:
- 0:xx  → 12:xx AM (midnight)
- 1-11:xx → 1-11:xx AM (morning)
- 12:xx → 12:xx PM (noon)
- 13-23:xx → 1-11:xx PM (afternoon/evening)

---

## HCS12 Assembly Notes

### Using Conditional Assembly

The HCS12 assembler supports conditional assembly:

```asm
IF SELECT12HOURS == 1
    ; Code compiled only if SELECT12HOURS == 1
    am_pm: DS.B 1      ; AM/PM byte only in 12h mode
ELSE
    ; Code compiled only if SELECT12HOURS == 0
    ; (no AM/PM byte in 24h mode)
ENDIF
```

### CodeWarrior Setup

1. **Set assembler symbol** in project settings:
   - Project → Edit Project → Assembler
   - Add symbol: `SELECT12HOURS=1`

2. **Or use preprocessor directive:**
   ```asm
   SELECT12HOURS: EQU 1
   ```

---

## Troubleshooting

### Display Shows Wrong Hours

**Problem:** Display shows `1:30 PM` instead of `01:30 PM`
- **Solution:** Check `snprintf` format specifier uses `%02d` (zero-padded)

### AM/PM Not Switching at Midnight

**Problem:** Display stays on AM or PM
- **Solution:** Verify internal hour rolls from 23 → 0, and format function checks `hour >= 12`

### Tests Report Failures

**Problem:** One or more tests fail
- **Solution:** 
  1. Verify `SELECT_12HOURS` is set correctly
  2. Check `clock_time_inc_second()` cascade logic
  3. Enable debug output to trace time values

### Button Doesn't Work

**Problem:** GPIO2 button doesn't toggle mode
- **Solution:**
  1. Verify GPIO2 is properly initialized
  2. Confirm `SELECT_12HOURS == 1` in code
  3. Check GPIO2 is connected to GND with pull-up
  4. Verify button debounce logic

---

## References

- **12-Hour Clock System:** https://en.wikipedia.org/wiki/12-hour_clock
- **HCS12 Assembler Guide:** CodeWarrior → Help → Assembler Reference → Conditional Assembly
- **Pico SDK Documentation:** https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf
- **C Conditional Compilation:** https://gcc.gnu.org/onlinedocs/cpp/Conditional-Syntax.html

---

## Support

For detailed implementation details, see `12H_MODE_DOCUMENTATION.md`

For HCS12 assembly implementation, see `clock_12h.asm`

---

**Last Updated:** 2025  
**Version:** 1.0  
**Mode Support:** 12-hour and 24-hour display
