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
double bias_z;
bool run;
int nanosecondInterval = 4000000;
double accelSensitivity = 9.81;
double minStationaryVal = 0, maxStationaryVal = 0;
double position = 0;
chrono::nanoseconds nsInterval(nanosecondInterval);
thread accelThread;
chrono::high_resolution_clock accelClock;
double secondsPerNSInterval = static_cast<double>(nanosecondInterval) / 1e9;

double convertToMPS(short accel)
{
    double temp = static_cast<double>(accel) / 2048 * accelSensitivity - bias_z;
    return between(temp, minStationaryVal, maxStationaryVal) ? 0 : temp;
}

double getAccelSamples(size_t samples, const char *fpath = nullptr, double *minVal = nullptr, double *maxVal = nullptr)
{
    // initialize variables
    position = 0;
    samplesTaken = 0;
    chrono::high_resolution_clock::time_point start;
    long long count;
    double accel = 0, pastAccel = 0, pastVelocity = 0, velocity = 0;
    double accumulator = 0;

    // log
    ofstream *out = nullptr;
    if (fpath != nullptr)
    {
        out = new ofstream(fpath);
    }

    // get min, max vals
    bool getMinMax = !(minVal == nullptr && maxVal == nullptr);

    while (samplesTaken < samples)
    {
        start = accelClock.now();

        // integration
        pastAccel = accel;
        accel = convertToMPS(accel_z());
        accumulator += accel;
        pastVelocity = velocity;
        velocity += (accel + pastAccel) * 0.5 * secondsPerNSInterval;
        position += (pastVelocity + velocity) * 0.5 * secondsPerNSInterval;
        ++samplesTaken;

        // check if fallen behind
        count = (accelClock.now() - start).count();
        if (count > nanosecondInterval)
        {
            cout << "fell behind!!!" << endl;
        }

        // log
        if (fpath != nullptr)
        {
            *out << "past Accel: " << pastAccel << ", accel: " << accel << ", past Velocity: "
                 << pastVelocity << ", velocity: " << velocity << ", position: " << position << endl;
        }

        // get min, max
        if (getMinMax)
        {
            *minVal = min(*minVal, accel);
            *maxVal = max(*maxVal, accel);
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

void getAccelVals()
{
    // initialize variables
    chrono::high_resolution_clock::time_point start;
    long long count;
    double accel = 0, pastAccel = 0, pastVelocity = 0, velocity = 0;

    while (run)
    {
        start = accelClock.now();

        // integration
        pastAccel = accel;
        accel = convertToMPS(accel_z());
        pastVelocity = velocity;
        velocity += (accel + pastAccel) * 0.5 * secondsPerNSInterval;
        position += (pastVelocity + velocity) * 0.5 * secondsPerNSInterval;
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

bool setupGetAccelVals()
{
    samplesTaken = 0;
    position = 0;
    run = true;
    return true;
}

bool cleanupGetAccelVals()
{
    run = false;
    accelThread.join();
    return true;
}

bool startGetAccelVals()
{
    accelThread = thread(getAccelVals);
    return true;
}

double getPosition()
{
    return position;
}

void calibAccel(size_t samples)
{
    bias_z = getAccelSamples(samples / 2) / samplesTaken;
    double minVal = numeric_limits<double>::max(), maxVal = numeric_limits<double>::min();
    getAccelSamples(samples / 2, nullptr, &minVal, &maxVal);
    minStationaryVal = minVal;
    maxStationaryVal = maxVal;
    cout << "accel calibrated in z direction with bias " << bias_z << " and min, max stationary vals of "
         << minStationaryVal << ", " << maxStationaryVal << endl;
}