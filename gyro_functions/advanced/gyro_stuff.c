#include <kipr/wombat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gyro_cpp.h"
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
#define TIME_TO_WAIT 10

double counts_till_360 = 0;
double gyro_bias = 0;

// ---------- calibraters ----------
/**
 * @brief calibrates the counts till 360
 *
 */
void gyro_calib_degrees()
{
    printf("Please turn the wombat 360 degrees. Waiting for %i seconds...\n", TIME_TO_WAIT);
    double start_time = seconds();

    // start accumulating
    setupGetGyroVals();
    startGetGyroVals();

    // wait
    while (seconds() - start_time < TIME_TO_WAIT)
    {
    }

    // cleanup
    cleanupGetGyroVals();

    printf("Waited 10 seconds. The total counts was %lf\n", getAccumulator());
    counts_till_360 = getAccumulator();
}
/**
 * @brief Function for easy calibration of the gyro. Calls
 * gyro_calib_bias and gyro_calib_degrees
 *
 */
void gyro_calib(size_t samples)
{
    calibrateGyro(samples);
    gyro_calib_degrees();
}

// ---------- managers for persistent data ----------
/**
 * @brief write the gyro_bias and counts_till_360 to a file
 *
 * @param fpath the name of the file
 */
void gyro_write_biases(const char *fpath)
{
    FILE *file = fopen(fpath, "wb");
    fwrite(&gyro_bias, sizeof(double), 1, file);
    fwrite(&counts_till_360, sizeof(double), 1, file);
    fclose(file);
}
/**
 * @brief Gets stored calibration data. Useful when you just need to retreive values
 *
 * @param fpath the name of the file
 */
void gyro_get_biases(const char *fpath)
{
    FILE *file = fopen(fpath, "rb");
    fread(&gyro_bias, sizeof(double), 1, file);
    fread(&counts_till_360, sizeof(double), 1, file);
    fclose(file);
    printf("gyro bias is %lf and counts till 360 is %lf", gyro_bias, counts_till_360);
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

    setupGetGyroVals();
    startGetGyroVals();

    while (seconds() - start_time < duration)
    {
        create_drive_direct(left_speed, right_speed);
        // correct errors
        // clockwise > 0, counterclockwise < 0
        if (getAccumulator() > 0)
        {
            right_speed = max(right_speed + 1, speed);
            left_speed = min(speed, left_speed - 1);
        }
        if (getAccumulator() < 0)
        {
            left_speed = max(left_speed + 1, speed);
            right_speed = min(speed, right_speed - 1);
        }
        msleep(1);
    }
    create_drive_direct(0, 0);
    cleanupGetGyroVals();
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
    setupGetGyroVals();
    startGetGyroVals();

    create_drive_direct(left_wheel_speed, right_wheel_speed);
    while (abs(getAccumulator()) < degrees * counts_till_360 / 360)
    {
    }
    create_drive_direct(0, 0);
    cleanupGetGyroVals();
}

int main()
{
    printf("hello yall");

    // calibrate
    int samples = 100;
    gyro_calib(samples);
    gyro_write_biases("biases.bin");

    printf("bias is %lf\n", getBias());

    getGyroSamples(samples);
    printf("average value was now %lf\n", getAccumulator() / samples);
    printf("number of samples taken was %i\n", getSamplesTaken());

    // read biases
    // gyro_get_biases("biases.bin");

    // use them in your program
    // create_connect_once();

    // drive_straight(10, 100);
    // turn_degrees(360, 100, -100);

    // create_disconnect();
    return 0;
}