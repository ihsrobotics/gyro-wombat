#include "gyro_cpp.h"

#include <kipr/wallaby.h>

#include <chrono>
#include <iostream>
#include <thread>

double accumulator;
double bias;
bool run;
int nanosecondInterval = 2;
std::chrono::nanoseconds nsInterval(nanosecondInterval);
std::thread gyroThread;
std::chrono::high_resolution_clock gyroClock;

void getGyroVals()
{
    std::chrono::high_resolution_clock::time_point start;
    while (run) {
        start = gyroClock.now();
        accumulator += gyro_z() - bias;
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
    bias = _bias;
    return true;
}

double getAccumulator()
{
    return accumulator;
}