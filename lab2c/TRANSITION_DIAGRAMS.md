# 12-Hour Clock Transition Diagrams

## Complete 12-Hour Cycle Visualization

```
                    12-HOUR CLOCK CYCLE
                         (24 hours)

    MIDNIGHT                  NOON                  MIDNIGHT
        ↓                       ↓                        ↓
    00:00:00              12:00:00                 24:00:00(=0:00)
    12:00 AM              12:00 PM                 12:00 AM
        │                       │                        │
        │  A M   P E R I O D    │   P M   P E R I O D    │
        │  (12 hours)           │   (12 hours)           │
        │                       │                        │
        ├──────────────────────┬┼────────────────────────┤
        │                      ││                        │
    00:00 AM             12:00 PM                   00:00 AM (next day)
    01:00 AM             01:00 PM
    02:00 AM             02:00 PM
    ...                   ...
    11:00 AM             11:00 PM
    11:59:59 AM          11:59:59 PM
```

## Critical Transition 1: Noon (AM → PM)

```
INTERNAL 24h FORMAT:
    11:59:59 ──[+1 sec]──→ 12:00:00
      ↓                         ↓
    11:59:59 AM          12:00:00 PM
   (displayed)           (displayed)

SECONDS cascade:    59 → 0 (roll over)
MINUTES cascade:    59 → 0 (roll over)
HOURS change:       11 → 12 (increment)
AM/PM toggle:       AM → PM (critical marker!)

Example in C:
    clock_time_set_utc(11, 59, 59);      // 11:59:59
    clock_time_inc_second(tick_us);      // +1 second
    // Result: 12:00:00 (displays as 12:00:00 PM)
```

## Critical Transition 2: Early Afternoon (PM continues)

```
INTERNAL 24h FORMAT:
    12:59:59 ──[+1 sec]──→ 13:00:00
      ↓                         ↓
    12:59:59 PM          01:00:00 PM
   (displayed)           (displayed)

SECONDS cascade:    59 → 0 (roll over)
MINUTES cascade:    59 → 0 (roll over)
HOURS change:       12 → 13 (increment)
Display hour:       12 → 1 (13 % 12 = 1)
AM/PM toggle:       PM → PM (stays PM)

Conversion Logic:
    hour = 13
    display_hour = 13 % 12 = 1
    am_pm = (13 >= 12) ? "PM" : "AM" = "PM"
    Result: "01:00:00 PM" ✓

Example in C:
    clock_time_set_utc(12, 59, 59);      // 12:59:59
    clock_time_inc_second(tick_us);      // +1 second
    // Result: 13:00:00 (displays as 01:00:00 PM)
```

## Critical Transition 3: Midnight (PM → AM, Day Wrap)

```
INTERNAL 24h FORMAT:
    23:59:59 ──[+1 sec]──→ 00:00:00
      ↓                         ↓
    11:59:59 PM          12:00:00 AM
   (displayed)           (displayed)
    
   END OF DAY          START OF NEW DAY

SECONDS cascade:    59 → 0 (roll over)
MINUTES cascade:    59 → 0 (roll over)
HOURS cascade:      23 → 0 (roll over, DAY WRAP!)
Display hour:       23 → 0, then 0 → 12 (0 % 12 = 0 → 12)
AM/PM toggle:       PM → AM (critical marker!)

Conversion Logic:
    hour = 0
    display_hour = 0 % 12 = 0
    if (0 == 0) display_hour = 12
    am_pm = (0 >= 12) ? "PM" : "AM" = "AM"
    Result: "12:00:00 AM" ✓

Example in C:
    clock_time_set_utc(23, 59, 59);      // 23:59:59
    clock_time_inc_second(tick_us);      // +1 second
    // Result: 00:00:00 (displays as 12:00:00 AM)
```

## Critical Transition 4: Early Morning (AM continues)

```
INTERNAL 24h FORMAT:
    00:59:59 ──[+1 sec]──→ 01:00:00
      ↓                         ↓
    12:59:59 AM          01:00:00 AM
   (displayed)           (displayed)

SECONDS cascade:    59 → 0 (roll over)
MINUTES cascade:    59 → 0 (roll over)
HOURS change:       0 → 1 (increment)
Display hour:       12 → 1 (conversion: 0→12, then 1→1)
AM/PM toggle:       AM → AM (stays AM)

Conversion Logic:
    hour = 1
    display_hour = 1 % 12 = 1
    if (1 == 0) false, keep 1
    am_pm = (1 >= 12) ? "PM" : "AM" = "AM"
    Result: "01:00:00 AM" ✓

Example in C:
    clock_time_set_utc(0, 59, 59);       // 00:59:59
    clock_time_inc_second(tick_us);      // +1 second
    // Result: 01:00:00 (displays as 01:00:00 AM)
```

## Cascade Overflow Flow Chart

```
                    CLOCK INCREMENT (+1 SECOND)
                              │
                              ↓
                    [ seconds++ ]
                              │
                         Yes /  \ No
                           /      \
        [seconds ≥ 60?] ─┘        └─→ DONE
                │
                ↓
        [ seconds = 0 ]
        [ minutes++   ]
                │
                ↓
            [minutes ≥ 60?]
              Yes /  \ No
                /      \
    ──────────┘        └─→ DONE
             │
             ↓
        [ minutes = 0 ]
        [ hours++     ]
             │
             ↓
          [hours ≥ 24?]
           Yes /  \ No
             /      \
  ──────────┘        └─→ DONE
           │
           ↓
      [ hours = 0 ]
      [ Day Wrap  ]
           │
           ↓
         DONE
```

## Display Conversion Flowchart

```
                INTERNAL 24h HOUR VALUE
                       │
                       ↓
                [hour % 12 = hour_12]
                       │
                       ↓
                [hour_12 == 0?]
              Yes /          \ No
                /              \
    [hour_12 = 12]           [keep hour_12]
        │                          │
        └──────────┬───────────────┘
                   ↓
            [hour_12 value]
                   │
                   ├─→ Can be 1-11: display as is
                   ├─→ Can be 12: special case (noon or midnight)
                   │
                   ↓
            [hour >= 12?]
              Yes /  \ No
                /      \
        [ "PM" ]       [ "AM" ]
            │              │
            └────┬──────────┘
                 ↓
         AM/PM INDICATOR
                 │
                 ├─→ Combined with hour_12
                 ├─→ Format: "HH:MM:SS AM/PM"
                 ↓
            DISPLAY OUTPUT
```

## Hour Conversion Matrix (Visual)

```
    MIDNIGHT ├─ 00 → 12 AM ─┤
    (00:00)  │  01 → 01 AM  │
             │  02 → 02 AM  │
             │  ...         │
             │  11 → 11 AM  │
             ├─ 12 → 12 PM  ┤ NOON
             │  13 → 01 PM  │
             │  14 → 02 PM  │
    (12:00)  │  ...         │
             │  23 → 11 PM  │
             └─────────────-┘
    
    PATTERN:
    - 00: Special case → 12 (midnight marker)
    - 01-11: Display as is (AM period)
    - 12: Special case → 12 (noon marker)
    - 13-23: Subtract 12 (PM period)
    
    FORMULA: display_hour = (hour % 12) || 12
    (i.e., if hour % 12 == 0, use 12, else use hour % 12)
```

## Minute-by-Minute Transition Example: 11:59:30 + 30 seconds

```
Time Increment Sequence (30 iterations of +1 second):

11:59:30 AM
11:59:31 AM
11:59:32 AM
11:59:33 AM
11:59:34 AM
11:59:35 AM
11:59:36 AM
11:59:37 AM
11:59:38 AM
11:59:39 AM
11:59:40 AM
11:59:41 AM
11:59:42 AM
11:59:43 AM
11:59:44 AM
11:59:45 AM
11:59:46 AM
11:59:47 AM
11:59:48 AM
11:59:49 AM
11:59:50 AM
11:59:51 AM
11:59:52 AM
11:59:53 AM
11:59:54 AM
11:59:55 AM
11:59:56 AM
11:59:57 AM
11:59:58 AM
11:59:59 AM          ← Last second before transition
    ↓
[CASCADE OVERFLOW]   ← seconds: 59→0, minutes: 59→0, hours: 11→12
    ↓
12:00:00 PM          ← Transition complete!

Critical Point: At 11:59:59 AM, the next increment causes ALL fields to change!
- seconds: 59 → 0 ✓
- minutes: 59 → 0 ✓
- hours: 11 → 12 ✓
- AM/PM: AM → PM ✓
```

## State Machine Diagram

```
                    ┌─────────────────┐
                    │   12:00:00 PM   │ NOON
                    │   (Hour = 12)   │ TRANSITION
                    └────────┬────────┘
                             │
                    ┌────────↓────────┐
                    │  11:59:59 AM    │
                    │   (Hour = 11)   │
                    └────────┬────────┘
                             │
                    ┌────────↓────────┐
                    │  01:00:00 AM    │
                    │   (Hour = 1)    │
                    └────────┬────────┘
                             │
                    ┌────────↓────────┐
                    │  00:59:59 AM    │
                    │   (Hour = 0)    │
                    └────────┬────────┘
                             │
                    ┌────────↓────────┐
                    │  11:59:59 PM    │
                    │   (Hour = 23)   │ MIDNIGHT
                    │  DAY BOUNDARY   │ TRANSITION
                    └────────┬────────┘
                             │
                    ┌────────↓────────┐
                    │  12:00:00 AM    │
                    │   (Hour = 0)    │
                    └────────┬────────┘
                             │
                    ┌────────↓────────┐
                    │  01:00:00 PM    │
                    │   (Hour = 13)   │
                    └────────┬────────┘
                             │
                    ┌────────↓────────┐
                    │  12:59:59 PM    │
                    │   (Hour = 12)   │
                    └────────┬────────┘
                             │
                    [back to noon]
```

## Test Coverage Diagram

```
     24-HOUR PERIOD
     ├─ MIDNIGHT (00:00-00:59)
     │  ├─ Test 4: 00:59:59 → 01:00:00 (early AM) ✓
     │  └─ Contains day boundary from previous day
     │
     ├─ MORNING (01:00-11:59)
     │  ├─ Standard AM hours
     │  └─ No special transitions
     │
     ├─ NOON (12:00-12:59)
     │  ├─ Test 1: 11:59:59 → 12:00:00 (AM→PM) ✓
     │  ├─ Display: 11:59:59 AM → 12:00:00 PM
     │  └─ Contains critical noon marker
     │
     ├─ AFTERNOON (13:00-23:59)
     │  ├─ Test 2: 12:59:59 → 13:00:00 (PM) ✓
     │  ├─ Display: 12:59:59 PM → 01:00:00 PM
     │  └─ Test 3: 23:59:59 → 00:00:00 (PM→AM) ✓
     │
     ├─ Test 5: Minute cascade ✓
     ├─ Test 6: Hour cascade + day wrap ✓
     └─ Test 7: Format strings ✓

ALL 4 CRITICAL TRANSITIONS VERIFIED ✓
```

---

## Summary

**The 4 Critical Transitions:**

1. ✓ **11:59:59 AM → 12:00:00 PM** (Noon begins)
2. ✓ **12:59:59 PM → 01:00:00 PM** (Afternoon continues)
3. ✓ **11:59:59 PM → 12:00:00 AM** (Midnight begins)
4. ✓ **12:59:59 AM → 01:00:00 AM** (Morning continues)

**Key Implementation Points:**
- Hours stored internally as 0-23 (24h format)
- Display conversion happens at format time (not in storage)
- Cascade logic ensures proper overflow sequencing
- Special handling for 0 and 12 (midnight and noon markers)
- AM/PM determined by hour >= 12

**All overflow paths fully tested and validated! ✓**
