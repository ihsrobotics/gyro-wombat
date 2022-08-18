#include <kipr/wallaby.h>
#include <stdio.h>
#include <stdlib.h>
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
#define between(a, b, c) ((b <= a && a <= c) || (b >= a && a >= c)) // check if a is between b and c

double gyro_bias = 0;
double accel_bias_x = 0;
double accel_bias_y = 0;
double accel_lowpass_x, accel_highpass_x, accel_lowpass_y, accel_highpass_y;

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

void accel_calib_bias(int samples)
{
    double total_x = 0;
    double total_y = 0;
    double min_x = 0, max_x = 0;
    double min_y = 0, max_y = 0;
    double accel_x_val, accel_y_val;
    for (int i = 0; i < samples; ++i)
    {
        accel_x_val = accel_x();
        accel_y_val = accel_y();
        min_x = min(min_x, accel_x_val);
        max_x = max(max_x, accel_x_val);
        min_y = min(min_y, accel_y_val);
        max_y = max(max_y, accel_y_val);
        total_x += accel_x_val;
        total_y += accel_y_val;
        msleep(1);
    }

    accel_bias_x = total_x / samples;
    accel_bias_y = total_y / samples;
    accel_highpass_x = max_x;
    accel_lowpass_x = min_x;
    accel_highpass_y = max_y;
    accel_lowpass_y = min_y;

    printf("finished with accel bias x %lf and accel bias y %lf\n", accel_bias_x, accel_bias_y);
    printf("min x val was %lf, max x val was %lf, min y val was %lf, max y val was %lf\n", min_x, max_x, min_y, max_y);
}

// ---------- movement functions ----------
/**
 * @brief Drives the roomba in a straight line for `duration` seconds.
 *
 * @param duration how long to drive forward for
 * @param speed speed at which to drive.
 */
void drive_straight()
{
    double duration = 2;
    double speed = 100;
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

// units -> meters : units / 9.8 = meters
int main()
{
    double pos_x = 0;
    double pos_y = 0;
    double vel_x = 0;
    double vel_y = 0;
    double accel_x_val, accel_y_val;

    gyro_calib_bias(100);
    accel_calib_bias(100);

    double delta_t;
    double start_time;
    double prev_time;
    double cur_time = prev_time = start_time = seconds();
    while ((cur_time = seconds()) - start_time < 3)
    {
        delta_t = cur_time - prev_time;
        accel_x_val = accel_x();
        accel_y_val = accel_y();
        if (between(accel_x_val, accel_lowpass_x, accel_highpass_x))
        {
            accel_x_val = 0;
        }
        else
        {
            accel_x_val -= accel_bias_x;
        }
        if (between(accel_y_val, accel_lowpass_y, accel_highpass_y))
        {
            accel_y_val = 0;
        }
        else
        {
            accel_y_val -= accel_bias_y;
        }

        vel_x += accel_x_val * delta_t;
        vel_y += accel_y_val * delta_t;

        pos_x += vel_x * delta_t;
        pos_y += vel_y * delta_t;

        prev_time = cur_time;
        if (accel_x_val != 0 || accel_y_val != 0)
        {
            printf("delta t is %lf, vel x is %lf, vel y is %lf, accel_x is %lf, accel_y is %lf\n", delta_t, vel_x, vel_y, accel_x_val, accel_y_val);
        }
    }

    printf("finished with %lf x counts and %lf y counts\n", pos_x, pos_y);
    printf("This is equal to %lf x meters and %lf y meters\n", pos_x / 9.81, pos_y / 9.81);
}