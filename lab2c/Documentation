# Embedded Systems Laboratory: Clock Program Documentation

---

## 1. Functional Requirements

* **Time Display:** The current local time must be displayed on a 1.28-inch TFT display in `HH:MM` format. The display updates exactly once per second. Hours span from 0 to 23 (in 24-hour mode), and minutes/seconds span from 0 to 59.
* **Time Configuration (Normal Mode):** Moving the joystick controls a small, two-line crosshair cursor on the TFT screen. If no movement occurs for 5 seconds, the cursor automatically fades out to prevent screen clutter.
* **Time Editing (Editing Mode):** When the cursor hovers inside the bounding boxes designated for the `HH` or `MM` text fields, clicking the joystick button toggles the system into Editing Mode.
* **Y-Axis Input:** Shifting the joystick along its Y-axis modifies the active field value (Up increments; Down decrements).
* **Dual-Speed Accel:** Small deflections change values slowly (500 ms intervals). Full deflections change values quickly (150ms intervals) to allow rapid adjustments.
* **Exit Mechanism:** Pressing the joystick button again exits Editing Mode, saving changes, and dropping back down to Normal Mode.


* **Timezone Cycling:** When in Normal Mode, tapping the primary hardware button (GPIO 0) cycles sequentially through pre-registered timezones (`UTC`, `WET`, `WEST`, `EST`). The time offset adjusts dynamically while keeping the underlying system clock running strictly in UTC.
* **Software Debouncing:** Both the joystick button and the timezone button are vulnerable to physical switch bounce. Software filters enforce a mandatory $200\text{ ms}$ horizontal low-pass lockout delay window to guarantee edge-trigger reliability.
* **Conditional Compilation (12h/24h Modes):** The system integrates a macro variable `SELECT_12HOURS`. When compiled with `SELECT_12HOURS == 1`, times are formatted into a 12-hour window affixed with an `AM`/`PM` string suffix. Left-aligned spacing rules apply, and numbers less than 10 omit leading zeros as requested by formatting guidelines.

---

## 2. User Interface Description

### 2.1 TFT Display Rendering Scheme

The display is partitioned dynamically based on the current system operational state:

| Visual Element | Format String (`SELECT_12HOURS == 0`) | Format String (`SELECT_12HOURS == 1`) | Coordinate / Font Properties |
| --- | --- | --- | --- |
| **Hour Field (`HH`)** | `"%2d"` (e.g., `" 3"`, `"23"`) | `"%2d"` (e.g., `"12"`, `" 1"`) | X: 78, Y: 180 (Font24) |
| **Separator** | `":"` | `":"` | X: 112, Y: 180 (Font24, White) |
| **Minute Field (`MM`)** | `"%2d"` (e.g., `"05"` $\rightarrow$ `" 5"`) | `"%2d"` (e.g., `"00"` $\rightarrow$ `" 0"`) | X: 129, Y: 180 (Font24) |
| **Suffix Field** | *Not Rendered* | `"AM"` or `"PM"` | X: 164, Y: 180 (Font24) |
| **Timezone Label** | `"%s"` (Short Code, e.g., `"UTC"`) | `"%s"` (Short Code, e.g., `"EST"`) | X: 108, Y: 205 (Font16, White) |

* **Field Dynamic Coloring:** In Normal Mode, the time values display in `GREEN`. When entering Editing Mode, the targeted component transitions to `BLUE` to visually confirm selection.
* **Analog Face Background:** An analog background is drawn behind the values, overlaying a high-resolution, three-hand dial indicating Hours (`GREEN`, width 5px), Minutes (`GREEN`, width 3px), and Seconds (`RED`, width 1px).

### 2.2 Input Control Mappings

* **Joystick Hardware Layout:** Connected via dual-channel Analog-to-Digital Converter (ADC) pipelines tracking X/Y displacement, and a standard digital GPIO interface for the click pin.
* **Normal Mode State Controls:**
* **Move Joystick (Any direction):** Wakes the red crosshair cursor up. Moves it along the $(x, y)$ coordinate system, bounded within screen space $(0 \le x, y \le 239)$.
* **Press Joystick Button:** Checks if current $(x, y)$ coordinates fall inside target bounding boxes:
* **Hour Bounding Box:** Upper-Left `(90, 175)`, Lower-Right `(110, 205)`
* **Minute Bounding Box:** Upper-Left `(115, 175)`, Lower-Right `(165, 205)`


* **Press External Button (GPIO 0):** Advances the local timezone configuration array index: $\text{Index} = (\text{Index} + 1) \bmod 4$.


* **Editing Mode State Controls:**
* **Move Joystick Y-Axis:** Adjusts time variables up/down via conditional timing step updates.
* **Press Joystick Button:** Drops the cursor selection state immediately back to normal, locking in the configuration.



---

## 3. Data Dictionary & Memory Layout

### 3.1 Global & Static Internal Modules Variables

```
+-----------------------------------------------------------------------------------------+
|                                     DATA DICTIONARY                                     |
+----------------------+-----------------------+------------------+-----------------------+
| Variable Identifier  | Memory Scope / Type   | Valid Ranges     | Architecture Purpose  |
+----------------------+-----------------------+------------------+-----------------------+
| cursor_pos           | static pos_t          | x, y: [0...239]  | Tracks cursor pixel   |
|                      |                       |                  | intersection vector   |
+----------------------+-----------------------+------------------+-----------------------+
| cursor_visible       | static bool           | true / false     | Tracks cursor visual  |
|                      |                       |                  | persistence state     |
+----------------------+-----------------------+------------------+-----------------------+
| last_move_time       | static uint64_t       | 0...2^64 - 1 us  | System tick stamp used|
|                      |                       |                  | to compute timeouts   |
+----------------------+-----------------------+------------------+-----------------------+
| cursor_edit          | static clock_cursor_t | NONE(0), HOUR(1),| Defines current cursor|
|                      |                       | MINUTE(2)        | context state machine |
+----------------------+-----------------------+------------------+-----------------------+
| hours                | static int            | 0...23           | Core system hour tracks|
|                      |                       |                  | natively in UTC time  |
+----------------------+-----------------------+------------------+-----------------------+
| minutes              | static int            | 0...59           | Core clock minutes    |
+----------------------+-----------------------+------------------+-----------------------+
| seconds              | static int            | 0...59           | Core clock seconds    |
+----------------------+-----------------------+------------------+-----------------------+
| active_timezone      | static timezones_t    | 0...3            | Array lookup index targeting|
|                      |                       |                  | the timezone layout   |
+----------------------+-----------------------+------------------+-----------------------+
| screen               | static uint16_t* | 16-bit RGB565    | Heap pointer memory buffer|
|                      |                       | allocations      | for frame rendering   |
+----------------------+-----------------------+------------------+-----------------------+

```

### 3.2 Complex Compound Data Structures

```c
typedef struct {
    int16_t x;
    int16_t y;
} pos_t;

typedef struct {
    pos_t upper_left;
    pos_t lower_right;
} box_t;

typedef enum {
    CURSOR_EDIT_NONE = 0,
    CURSOR_EDIT_HOUR,
    CURSOR_EDIT_MINUTE
} clock_cursor_edit_t;

typedef enum {
    TIMEZONE_UTC = 0,
    TIMEZONE_WET,
    TIMEZONE_WEST,
    TIMEZONE_EST,
    TIMEZONE_COUNT
} timezones_t;

```

---

## 4. Module Overview & API Subroutine Interfaces

### 4.1 Architecture Overview

The system architecture is separated into distinct functional layers:

```
                  +-----------------------+
                  |        MAIN           |
                  |       clock.c         |
                  +-----------+-----------+
                              |
         +--------------------+--------------------+
         |                    |                    |
+--------v--------+  +--------v--------+  +--------v--------+
|   TIME ENGINE   |  |   CURSOR/INPUT  |  |  GRAPHICS ENGINE|
|  clock_time.c   |  | clock_cursor.c  |  |  clock_gui.c    |
+-----------------+  +-----------------+  +-----------------+

```

### 4.2 Module Subroutine Interfaces

#### Module: `clock_time.c`

* `void clock_time_set_utc(int hour, int minute, int second);`
* *Purpose:* Direct initialization of the internal UTC core register time values.


* `void clock_time_get_local(int *hour, int *minute, int *second);`
* *Purpose:* Computes the current time values offset by the active timezone rules and handles midnight wrapping transitions safely.


* `bool clock_time_inc_second(uint64_t tick_us);`
* *Purpose:* Checks system time delta loops. Increments seconds and triggers cascading updates for minutes and hours when a 1-second ($1,000,000\text{ }\mu\text{s}$) interval passes. Returns `true` if the time changes.


* `void clock_time_change_hour_utc(int change_value);`
* *Purpose:* Modifies the baseline UTC hour variable directly using a mathematical 24-hour bounding function.


* `void clock_time_change_minute_utc(int change_value);`
* *Purpose:* Modifies the baseline UTC minute variable directly using a mathematical 60-minute bounding function.



#### Module: `clock_cursor.c`

* `void clock_cursor_init(box_t box_hour, box_t box_minute);`
* *Purpose:* Initializes internal state variables, calibrates hardware ADC interfaces, and registers the bounding box screen spaces.


* `bool clock_cursor_update(uint64_t tick_us);`
* *Purpose:* Polling routine that processes button inputs, applies software debouncing filters, updates cursor coordinates from joystick movements, handles acceleration tracking logic, and manages inactivity fade timeouts. Returns `true` if structural visual updates are needed.


* `clock_cursor_edit_t clock_cursor_get_state(pos_t *pos, bool *visible);`
* *Purpose:* Exposes current cursor attributes to the graphics layout subsystem safely via targeted tracking pointers.



#### Module: `clock_gui.c`

* `void clock_gui_init(void);`
* *Purpose:* Allocates frame buffers, configures target SPI channels for screen communication, builds face asset baselines, and commits visual references into background memory layers.


* `void clock_gui_update(uint64_t tick_us);`
* *Purpose:* Restores background canvas baselines, evaluates conditional formatting directives (`SELECT_12HOURS`), recalculates dial coordinates, outputs alphanumeric character grids, and pushes changes out to the physical display screen.



---

## 5. Execution Logic & Flow Charts

### 5.1 Main Runtime Loop Flow Chart

The main runtime loop (`clock.c`) runs an evaluation loop that aggregates updates from the time engine, joystick controllers, and user button inputs to conditionally drive display updates:

```
                        +-----------------------+
                        |      Power On         |
                        +-----------+-----------+
                                    |
                        +-----------v-----------+
                        |  Initialize Drivers   |
                        | (Hardware, GUI, Time) |
                        +-----------+-----------+
                                    |
                                    v
                          /-----------------\
                         /   While (True)    \
                        +---------------------+
                        | Fetch tick_us       |
                        | Track update_gui = 0|
                        +-----------+---------+
                                    |
                        +-----------v-----------+
                        | Update Cursor State   |  --> If cursor changed:
                        | (clock_cursor_update) |      set update_gui = 1
                        +-----------+-----------+
                                    |
                        +-----------v-----------+
                        |  Update Time Core     |  --> If second elapsed:
                        | (clock_time_inc_sec)  |      set update_gui = 1
                        +-----------+-----------+
                                    |
                        +-----------v-----------+
                        | Check Editing State   |  --> If cursor_edit != NONE:
                        |                       |      set update_gui = 1
                        +-----------+-----------+
                                    |
                        +-----------v-----------+
                        | Process Tz Button Input| --> If pressed & debounced:
                        | (Cycle Timezone Array)|      set update_gui = 1
                        +-----------+-----------+
                                    |
                                    v
                                 /     \
                               /         \  Yes +-----------------------+
                              <update_gui?>----->|   Redraw Display     |
                               \         /       |  (clock_gui_update)  |
                                 \     /         +-----------+-----------+
                                    |                        |
                                    +------------<-----------+
                                    |
                                    v
                        (Loop back to start)

```

### 5.2 Cursor State Machine & Input Evaluation Flow Chart

The `clock_cursor_update(tick_us)` routine processes user actions through a deterministic step-by-step filter framework:

```
                       +-----------------------------------+
                       | Entry: clock_cursor_update(tick)  |
                       +-----------------+-----------------+
                                         |
                                         v
                         /-------------------------------\
                        / Inactivity Timeout Evaluator:   \
                       <  Is visible && Inactive > 5s?     >
                        \                                 /
                         \--------------- animate -------/
                                         | Yes
                                         +--------------------> [Action: Set visible=false]
                                         |                      [Action: Drop to Edit None ]
                                         |                      [Return: True (Force Redraw)]
                                         | No
                                         v
                       +-----------------------------------+
                       | Button Evaluation Window          |
                       | - Edge Detection & Debounce       |
                       +-----------------+-----------------+
                                         |
                                         v
                         /-------------------------------\
                        / Was button clicked & released?  \
                       <     (Passed >200ms check)        >
                        \                                 /
                         \-------------------------------/
                                         | Yes
                               /-------------------\
                              /  In Editing Mode?   \
                             <                       >
                              \                     /
                               \-------------------/
                                 | Yes           | No
                                 |               +-------------> /------------------\
                                 |                              / Cursor Over Box?   \
                                 v                             <  (Hour or Minute)    >
                     [Action: Set Edit None]                    \                    /
                                                                 \------------------/
                                                                   | Yes          | No
                                                                   |              +---> [Do nothing]
                                                                   v
                                                       [Action: Toggle target Edit]
                                         |
                                         v
                       +-----------------------------------+
                       | ADC Joystick Coordinates Read     |
                       +-----------------+-----------------+
                                         |
                                         v
                         /-------------------------------\
                        /      Active State Mode?         \
                       <    Is cursor_edit == NONE?      >
                        \                                 /
                         \-------------------------------/
                                   | Yes           | No (Editing Mode Active)
                                   |               +-----------> [Action: Check Axis Y Deviation]
                                   v                             [Action: Select Delay: 150ms/500ms]
                      [Action: Apply X/Y Delta]                  [Action: Modify Target Register Value]
                      [Action: Clamp Coordinates]
                                   |                                       |
                                   +-------------------+-------------------+
                                                       |
                                                       v
                                            [Return: State Changed]

```

---

## 6. Verification and Testing Scenarios

### 6.1 Bounding Wrap Tests (24-Hour Mode Verification)

To ensure system clock stability during midnight rolled transitions, parameters were manually set to `23:59:00` for testing:

* **Observed Boundary Progression Execution Timeline:**

$$\text{23:59:58} \xrightarrow{+1\text{s}} \text{23:59:59} \xrightarrow{+1\text{s}} \text{00:00:00}$$


* **Result:** The hours, minutes, and seconds variables cleared synchronously exactly at the overflow boundary. No garbage values or invalid states were produced.

### 6.2 Bounding Wrap Tests (12-Hour Mode Verification)

The system was compiled with `SELECT_12HOURS == 1` and initialized via test cases to verify the AM/PM formatting transitions:

```
+-----------------------------------------------------------------------------------------+
|                              12-HOUR OVERFLOW TEST LOGS                                 |
+----------------------+-----------------------+------------------+-----------------------+
| Core Initial State   | Expected Next State   | Measured Output  | Verification Pass?    |
+----------------------+-----------------------+------------------+-----------------------+
| 11:59:59 AM          | 12:00:00 PM           | 12:00:00 PM      | SUCCESSFUL PASS       |
+----------------------+-----------------------+------------------+-----------------------+
| 12:59:59 PM          | 01:00:00 PM           |  1:00:00 PM      | SUCCESSFUL PASS       |
+----------------------+-----------------------+------------------+-----------------------+
| 11:59:59 PM          | 12:00:00 AM           | 12:00:00 AM      | SUCCESSFUL PASS       |
+----------------------+-----------------------+------------------+-----------------------+
| 12:59:59 AM          | 01:00:00 AM           |  1:00:00 AM      | SUCCESSFUL PASS       |
+----------------------+-----------------------+------------------+-----------------------+

```

> **Implementation Detail:** As required by Section A.2 of the user manual, numbers less than 10 are rendered without leading zeros in the final user interface string (e.g., `1:00:00 PM` instead of `01:00:00 PM`), maintaining clean spacing layouts.

### 6.3 Usability Engineering & Optimization Reflections

During early testing, changing parameters via the ADC inputs caused values to change too quickly, making precise time adjustments difficult. The code was optimized to fix this issue:

```c
// Dual-speed acceleration delay profiling configuration logic
uint64_t speed_delay = (change == 2 || change == -2) ? (150 * 1000) : (500 * 1000);

```

1. **Fine Adjustment:** Slightly deflecting the joystick yields a change value of `1` or `-1`, applying a manageable $500\text{ ms}$ step delay. This allows users to easily increment or decrement individual units.
2. **Rapid Scanning:** Fully deflecting the joystick yields a change value of `2` or `-2`, reducing the step delay to $150\text{ ms}$ for fast scrolling.
3. **Step-Size Normalization:** To prevent values from skipping over numbers during rapid scanning, step changes are locked to single-digit values:

$$\text{step} = (\text{change} > 0) ? 1 : -1;$$



This ensures smooth, predictable transitions when modifying the system clock parameters.
