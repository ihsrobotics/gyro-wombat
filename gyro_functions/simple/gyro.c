#include <kipr/wombat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

// adjust this number if it over or underturns
// if it underturns, make this greater
// if it overturns, make this smaller
double counts_till_360 = 1610000;
double gyro_bias = 0;

// ---------- calibraters ----------
/**
 * @brief Calibrates the gyro for sampling.
 *
 * @param samples the number of samples to take
 */
void gyro_calib_bias(int samples)
{
    double total = 0;
    for (int i = 0; i < samples; ++i) {
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
    double start_time = seconds();
    int left_speed = speed;
    int right_speed = speed;

    double accumulator = 0;
    int correction = 1;

    while (seconds() - start_time < duration) {
        create_drive_direct(left_speed, right_speed);
        accumulator += gyro_z() - gyro_bias;
        // correct errors
        if (accumulator > 0) {
            right_speed = max(right_speed + correction, speed);
            left_speed = min(speed, left_speed - correction);
        }
        if (accumulator < 0) {
            left_speed = max(left_speed + correction, speed);
            right_speed = min(speed, right_speed - correction);
        }
        msleep(1);
    }
    create_drive_direct(0, 0);
}

/**
 * @brief Turns `degrees` degrees. Note that, it will stop as soon as abs(rotation) is equal
 * to the desired turn. For example, if you wanted to turn 360 degrees right and you picked
 * up the wombat and turned it 360 degrees left, it will stop because it reached its desired
 * delta degrees.
 *
 * @param degrees number of degrees to turn.
 * @param left_wheel_speed speed for the left wheel
 * @param right_wheel_speed speed for the right wheel
 */
void turn_degrees(double degrees, int left_wheel_speed, int right_wheel_speed)
{
    double accumulator = 0;

    create_drive_direct(left_wheel_speed, right_wheel_speed);
    while (abs(accumulator) < degrees * counts_till_360 / 360) {
        accumulator += gyro_z() - gyro_bias;
        msleep(1);
    }
    create_drive_direct(0, 0);
}

int main()
{
    printf("hello yall");
    // calibrate
    gyro_calib_bias(10);

    // use them in your program
    create_connect_once();

    drive_straight(10, 100);
    turn_degrees(360, 100, -100);

    create_disconnect();
    return 0;
}