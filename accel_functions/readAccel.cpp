#include <kipr/wallaby.h>

#include <chrono>
#include <iostream>
#include <thread>

double accumulator;
size_t samplesTaken = 0;
double bias_z;
bool run;
int nanosecondInterval = 2;
std::chrono::nanoseconds nsInterval(nanosecondInterval);
std::thread accelThread;
std::chrono::high_resolution_clock accelClock;

void getAccelVals(size_t samples)
{
    std::chrono::high_resolution_clock::time_point start;
    while (samplesTaken < samples) {
        start = accelClock.now();
        accumulator += static_cast<double>(accel_z()) / (1 << 15) - bias_z;
        ++samplesTaken;
        std::this_thread::sleep_until(start + nsInterval);
    }
}

void getAccelVals()
{
    std::chrono::high_resolution_clock::time_point start;
    while (run) {
        start = accelClock.now();
        accumulator += static_cast<double>(accel_z()) / (1 << 15) - bias_z;
        ++samplesTaken;
        std::this_thread::sleep_until(start + nsInterval);
    }
}

bool setupGetAccelVals()
{
    accumulator = 0;
    samplesTaken = 0;
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
    accelThread = std::thread(getAccelVals);
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

size_t getSamplesTaken()
{
    return samplesTaken;
}

void calibAccel(size_t samples)
{
    accumulator = 0;
    samplesTaken = 0;
    getAccelVals(samples);
    bias_z = accumulator / samplesTaken;
    std::cout << "accel calibrated in x, y, z direction" << std::endl;
}