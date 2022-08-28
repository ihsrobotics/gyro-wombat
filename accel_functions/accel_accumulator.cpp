#include "accel_accumulator.h"
#include <kipr/wombat.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <fstream>
#include <algorithm>
#include <limits>
#ifndef between
#define between(val, _min, _max) (val >= _min && val <= _max)
#endif
using namespace std;

double accumulator;
size_t samplesTaken = 0;
double bias_y = 0, bias_x = 0;
bool run;
int nanosecondInterval = 4000000;
double accelSensitivity = 9.81;
double minStationaryYVal = 0, maxStationaryYVal = 0, minStationaryXVal = 0, maxStationaryXVal = 0;
double posX = 0, posY = 0;
chrono::nanoseconds nsInterval(nanosecondInterval);
thread accelThread;
chrono::high_resolution_clock accelClock;
double secondsPerNSInterval = static_cast<double>(nanosecondInterval) / 1e9;

double convertToMPSY(short accel)
{
    double temp = static_cast<double>(accel) / 2048 * accelSensitivity - bias_y;
    return between(temp, minStationaryYVal, maxStationaryYVal) ? 0 : temp;
}
double convertToMPSX(short accel)
{
    double temp = static_cast<double>(accel) / 2048 * accelSensitivity - bias_x;
    return between(temp, minStationaryXVal, maxStationaryXVal) ? 0 : temp;
}

double getPositionX()
{
    return posX;
}
double getPositionY()
{
    return posY;
}

/**
 * @brief Get samples from the accelerometer
 * @details Can also be used to log samples and
 * get the min, max Y and X accel vals. Sets `posX`, `posY`,
 * and `samplesTaken` to 0 before doing anything.
 *
 * @param samples number of samples to take
 * @param fpath the path to log to. By default, is nullptr (no logs)
 * @param[out] minValY a pointer to the min Y value, by default is nullptr
 * (do not store the min Y value)
 * @param[out] maxValY a pointer to the max Y value, by default is nullptr
 * (do not store the max Y value)
 * @param[out] minValX a pointer to the min X value, by default is nullptr
 * (do not store the min X value)
 * @param[out] maxValX a pointer to the max X value, by default is nullptr
 * (do not store the max X value)
 * @param[out] accumulatorY a pointer to the accumulator for y accel values,
 * by default is nullptr (do not accumulate)
 * @param[out] accumulatorX a pointer to the accumulator for x accel values,
 * by default is nullptr (do not accumulate)
 * @return double
 */
double getAccelSamples(size_t samples, const char *fpath = nullptr,
                       double *minValY = nullptr, double *maxValY = nullptr,
                       double *minValX = nullptr, double *maxValX = nullptr,
                       double *accumulatorY = nullptr, double *accumulatorX = nullptr)
{
    // initialize variables
    posX = 0;
    posY = 0;
    samplesTaken = 0;
    chrono::high_resolution_clock::time_point start;
    long long count;
    double accelY = 0, pastAccelY = 0, pastVelocityY = 0, velocityY = 0;
    double accelX = 0, pastAccelX = 0, pastVelocityX = 0, velocityX = 0;

    // log
    ofstream *out = nullptr;
    if (fpath != nullptr)
    {
        out = new ofstream(fpath);
    }

    // get min, max vals
    bool getMinMax = !(minValY == nullptr && maxValY == nullptr && minValX == nullptr && minValX == nullptr);

    // accumulate
    bool accumulate = !(accumulatorX == nullptr && accumulatorY == nullptr);

    while (samplesTaken < samples)
    {
        start = accelClock.now();

        // y integration
        pastAccelY = accelY;
        accelY = convertToMPSY(accel_y());
        pastVelocityY = velocityY;
        velocityY += (accelY + pastAccelY) * 0.5 * secondsPerNSInterval;
        posY += (pastVelocityY + velocityY) * 0.5 * secondsPerNSInterval;
        // x integration
        pastAccelX = accelX;
        accelX = convertToMPSX(accel_x());
        pastVelocityX = velocityX;
        velocityX += (accelX + pastAccelX) * 0.5 * secondsPerNSInterval;
        posX += (pastVelocityX + velocityX) * 0.5 * secondsPerNSInterval;
        ++samplesTaken;

        // accumulate
        if (accumulate)
        {
            *accumulatorY += accelY;
            *accumulatorX += accelX;
        }

        // log
        if (fpath != nullptr)
        {
            *out << "----------------------" << endl;
            *out << "past AccelY: " << pastAccelY << ", accelY: " << accelY << ", past VelocityY: "
                 << pastVelocityY << ", velocityY: " << velocityY << ", posY: " << posY << endl;
            *out << "past AccelX: " << pastAccelX << ", accelX: " << accelX << ", past VelocityX: "
                 << pastVelocityX << ", velocityX: " << velocityX << ", posX: " << posX << endl;
        }

        // get min, max
        if (getMinMax)
        {
            *minValY = min(*minValY, accelY);
            *maxValY = max(*maxValY, accelY);
            *minValX = min(*minValY, accelX);
            *maxValX = max(*maxValY, accelX);
        }

        // check if fallen behind
        count = (accelClock.now() - start).count();
        if (count > nanosecondInterval)
        {
            cout << "fell behind!!!" << endl;
        }

        // sleep
        this_thread::sleep_for(chrono::nanoseconds(nanosecondInterval - count));
    }

    // cleanup logs
    if (fpath != nullptr)
    {
        out->close();
        delete out;
    }

    return accumulator;
}

void printAccelSamples(const char *fpath, size_t samples)
{
    getAccelSamples(samples, fpath);
}

/**
 * @brief Get values from the accelerometer
 * @details This is meant to be run in the backgroun
 * on a separate thread. Does not set `posX` and `posY` to
 * 0 (so this should be done beforehand).
 * Does not set `samplesTaken` to 0 either.
 *
 */
void getAccelVals()
{
    // initialize variables
    chrono::high_resolution_clock::time_point start;
    long long count;
    double accelY = 0, pastAccelY = 0, pastVelocityY = 0, velocityY = 0;
    double accelX = 0, pastAccelX = 0, pastVelocityX = 0, velocityX = 0;

    while (run)
    {
        start = accelClock.now();

        // y integration
        pastAccelY = accelY;
        accelY = convertToMPSY(accel_y());
        pastVelocityY = velocityY;
        velocityY += (accelY + pastAccelY) * 0.5 * secondsPerNSInterval;
        posY += (pastVelocityY + velocityY) * 0.5 * secondsPerNSInterval;
        // x integration
        pastAccelX = accelX;
        accelX = convertToMPSX(accel_x());
        pastVelocityX = velocityX;
        velocityX += (accelX + pastAccelX) * 0.5 * secondsPerNSInterval;
        posX += (pastVelocityX + velocityX) * 0.5 * secondsPerNSInterval;
        ++samplesTaken;

        // check if fallen behind
        count = (accelClock.now() - start).count();
        if (count > nanosecondInterval)
        {
            cout << "fell behind!!!" << endl;
        }

        this_thread::sleep_for(chrono::nanoseconds(nanosecondInterval - count));
    }
}

/**
 * @brief should be done before using startGetAccelVals
 *
 * @return true
 * @return false
 */
bool setupGetAccelVals()
{
    samplesTaken = 0;
    posX = 0;
    posY = 0;
    run = true;
    return true;
}

/**
 * @brief Basically, stop getting accel vals
 *
 * @return true
 * @return false
 */
bool cleanupGetAccelVals()
{
    run = false;
    accelThread.join();
    return true;
}

/**
 * @brief Starts the thread that runs getAccelVals
 *
 * @return true
 * @return false
 */
bool startGetAccelVals()
{
    accelThread = thread(getAccelVals);
    return true;
}

/**
 * @brief calibrate the accelerometer (sets up biases, low and high passes)
 *
 * @param samples the number of samples to take.
 */
void calibAccel(size_t samples)
{
    cout << "calibrating accel..." << endl;
    cout << "with current rate of 1 sample every " << secondsPerNSInterval << " seconds, this will take "
         << samples * secondsPerNSInterval << " seconds." << endl;

    // get biases
    getAccelSamples(samples / 2, nullptr, nullptr, nullptr, nullptr, nullptr, &bias_y, &bias_x) / samplesTaken;
    double minYVal = numeric_limits<double>::max(), maxYVal = numeric_limits<double>::min();
    double minXVal = numeric_limits<double>::max(), maxXVal = numeric_limits<double>::min();
    getAccelSamples(samples / 2, nullptr, &minYVal, &maxYVal, &minXVal, &maxXVal);

    // set high/low passes
    minStationaryYVal = minYVal;
    maxStationaryYVal = maxYVal;
    minStationaryXVal = minXVal;
    maxStationaryXVal = maxXVal;

    // finish up
    cout << "accel calibrated in y direction with bias " << bias_y << " and min, max stationary vals of "
         << minStationaryYVal << ", " << maxStationaryYVal << endl;
    cout << "accel calibrated in x direction with bias " << bias_x << " and min, max stationary vals of "
         << minStationaryXVal << ", " << maxStationaryXVal << endl;
}