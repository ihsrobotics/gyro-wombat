// TODO - redo this to reflect accel_accumulator.cpp
#include <kipr/wallaby.h>
#include <stdio.h>
#include <stdlib.h>
#include "accel_accumulator.h"

// ---------- calibraters ----------
/**
 * @brief Calibrates the gyro for sampling.
 *
 * @param samples the number of samples to take
 */
void gyro_calib_bias(int samples)
{
    double total = 0;
    for (int i = 0; i < samples; ++i)
    {
        total += gyro_z();
        msleep(1);
    }
    gyro_bias = total / samples;
    printf("finished with bias %lf\n", gyro_bias);
}

// ---------- movement functions ----------
/**
 * @brief Drives the roomba in a straight line for `duration` seconds.
 *
 * @param duration how long to drive forward for
 * @param speed speed at which to drive.
 */
void drive_straight(double duration, int speed)
{
    msleep(100);
    double start_time = seconds();
    int left_speed = speed;
    int right_speed = speed;

    double accumulator = 0;
    int correction = 1;

    while (seconds() - start_time < duration)
    {
        create_drive_direct(left_speed, right_speed);
        accumulator += gyro_z() - gyro_bias;
        // correct errors
        if (accumulator > 0)
        {
            right_speed = max(right_speed + correction, speed);
            left_speed = min(speed, left_speed - correction);
        }
        if (accumulator < 0)
        {
            left_speed = max(left_speed + correction, speed);
            right_speed = min(speed, right_speed - correction);
        }
        msleep(1);
    }
    create_drive_direct(0, 0);
}

int main()
{
    // calibrate
    gyro_calib_bias(100);
    calibAccel(400);

    // go
    setupGetGyroVals();
    startGetAccelVals();
    drive_straight(2, 100);
    cleanupGetAccelVals();

    // check results
    printf("finished at x: %lf, y: %lf\n", getPositionX(), getPositionY());
    return 0;
}