#include "gyro_cpp.h"

#include <kipr/wallaby.h>

#include <chrono>
#include <iostream>
#include <thread>

double accumulator;
size_t samplesTaken;
double bias_z;
double theta;
bool run;
int nanosecondInterval = 2;
std::chrono::nanoseconds nsInterval(nanosecondInterval);
std::thread gyroThread;
std::chrono::high_resolution_clock gyroClock;

void getAccelVals(size_t samples)
{
    std::chrono::high_resolution_clock::time_point start;
    while (samplesTaken < samples) {
        start = accelClock.now();
        accumulator += static_cast<double>(gyro_z()) / (1 << 15) - bias_z;
        ++samplesTaken;
        std::this_thread::sleep_until(start + nsInterval);
    }
}

void getGyroVals()
{
    std::chrono::high_resolution_clock::time_point start;
    while (run) {
        start = gyroClock.now();
        double degreesPerSecond = static_cast<double>(gyro_z()) / (1 << 15) - bias_z;

        std::this_thread::sleep_until(start + nsInterval);
    }
}
bool setupGetGyroVals()
{
    accumulator = 0;
    run = true;
    return true;
}

bool cleanupGetGyroVals()
{
    run = false;
    gyroThread.join();
    return true;
}

bool startGetGyroVals()
{
    gyroThread = std::thread(getGyroVals);
    return true;
}

bool setBias(double _bias)
{
    bias_z = _bias;
    return true;
}

double getAccumulator()
{
    return accumulator;
}

void calibrateGyro(size_t samples)
{
    accumulator = 0;
    samplesTaken = 0;
    getGyroVals(samples);
    bias_z = accumulator / samplesTaken;
    std::cout << "accel calibrated in x, y, z direction" << std::endl;
}