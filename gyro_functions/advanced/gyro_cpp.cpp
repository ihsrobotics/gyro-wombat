#include "gyro_cpp.h"

#include <kipr/wallaby.h>

#include <chrono>
#include <iostream>
#include <thread>

double accumulator;
size_t samplesTaken;
double bias_z = 0;
double theta;
bool run;
int nanosecondInterval = 500000;
int gyroSensitivity;
std::chrono::nanoseconds nsInterval(nanosecondInterval);
std::thread gyroThread;
std::chrono::high_resolution_clock gyroClock;

void getGyroSamples(size_t samples)
{
    accumulator = 0;
    std::chrono::high_resolution_clock::time_point start;
    long long count;
    while (samplesTaken < samples)
    {
        start = gyroClock.now();
        accumulator += static_cast<double>(gyro_z()) / (1 << 15) * gyroSensitivity - bias_z;
        ++samplesTaken;
        count = (gyroClock.now() - start).count();
        if (count > nanosecondInterval)
        {
            std::cout << "fell behind!! at " << gyroClock.now().time_since_epoch().count() << std::endl;
            return;
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(nanosecondInterval - count));
    }
}

void getGyroVals()
{
    std::chrono::high_resolution_clock::time_point start;
    long long count;
    while (run)
    {
        start = gyroClock.now();
        double degreesPerSecond = static_cast<double>(gyro_z()) / (1 << 15) * gyroSensitivity - bias_z;
        count = (gyroClock.now() - start).count();
        if (count > nanosecondInterval)
        {
            std::cout << "fell behind!!" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(nanosecondInterval - count));
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

double getBias()
{
    return bias_z;
}

double getAccumulator()
{
    return accumulator;
}

void calibrateGyro(size_t samples)
{
    setup_gyro_sensitivity();
    gyroSensitivity = get_gyro_sensitivity();
    accumulator = 0;
    samplesTaken = 0;
    getGyroSamples(samples);
    setBias(accumulator / samplesTaken);
    std::cout << "gyro calibrated in z direction" << std::endl;
}

int getSamplesTaken()
{
    return samplesTaken;
}