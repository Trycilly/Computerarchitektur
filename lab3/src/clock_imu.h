#ifndef __CLOCK_IMU_H__
#define __CLOCK_IMU_H__

#include <stdbool.h>

// Enum for the tilt direction, which can be returned by the clock_imu_get_tilt() function
typedef enum {
    TILT_NONE = 0,
    TILT_LEFT,
    TILT_RIGHT
} clock_tilt_dir_t;

/**
 * Initializes the IMU sensor. Returns true if initialization was successful, false otherwise.
 */
bool clock_imu_init(void);

/**
 * Reads the current tilt direction from the IMU sensor and returns it as a clock_tilt_dir_t enum value.
 */
clock_tilt_dir_t clock_imu_get_tilt(void);

#endif /* __CLOCK_IMU_H__ */