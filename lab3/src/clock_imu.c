#include "clock_imu.h"
#include <stdio.h>

// Include the hardware driver for the IMU
#include "QMI8658.h" 

bool clock_imu_init(void) {
    // Initialize the IMU sensor
    QMI8658_init();
    return true;
}

clock_tilt_dir_t clock_imu_get_tilt(void) {
    float acc[3] = {0.0f, 0.0f, 0.0f};  // Array to hold X, Y, Z acceleration
    float gyro[3] = {0.0f, 0.0f, 0.0f}; // Array to hold gyroscope data (required by the read function)
    unsigned int tim;                   // Timestamp variable (required by the read function)
    
    // Read the actual sensor data into our arrays
    QMI8658_read_xyz(acc, gyro, &tim);
    
    // DEBUGGING TIP: If the screen isn't switching, uncomment the line below 
    // to see exactly what numbers your sensor is outputting in the serial console!
    // printf("X: %f, Y: %f, Z: %f\n", acc[0], acc[1], acc[2]);
    
    // Evaluate the X-axis for tilt. 
    // Depending on the driver, 1G of gravity might be represented as 1.0, 9.8, or 1000.
    // 500.0 is a safe threshold if it outputs in 'mg' (milli-g's, where 1000 = 90 degrees).
    // If your driver outputs in m/s^2, change these 500.0 values to 5.0.
    if (acc[0] < -500.0f) {
        return TILT_LEFT;
    } else if (acc[0] > 500.0f) {
        return TILT_RIGHT;
    }
    
    return TILT_NONE;
}