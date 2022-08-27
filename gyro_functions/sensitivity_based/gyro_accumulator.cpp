//#include "/mnt/c/users/eliot/documents/github/libwallaby/include/wallaby/wombat.h"
#include <kipr/wombat.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <algorithm>
#include <limits>
#include "gyro_accumulator.h"
#define between(val, _min, _max) (val >= _min && val <= _max)
using namespace std;
using namespace chrono;

double minStationaryVal = 0, maxStationaryVal = 0;
double accumulator = 0;
size_t samplesTaken = 0;
double bias_z = 0;
double theta = 0;
bool run = 0;
int nanosecondInterval = 4000000;
int gyroSensitivity;
std::chrono::nanoseconds nsInterval(nanosecondInterval);
std::thread gyroThread;
std::chrono::high_resolution_clock gyroClock;
double secondsPerNSInterval = static_cast<double>(nanosecondInterval) / 1e9;

double convertToDPS(short gyro)
{
    double temp = static_cast<double>(gyro) / 2048 * 250 - bias_z;
    return between(temp, minStationaryVal, maxStationaryVal) ? 0 : temp;
}

void getGyroSamples(size_t samples, const char *fpath, double *minVal = nullptr, double *maxVal = nullptr)
{
    samplesTaken = 0;
    theta = 0;
    std::chrono::high_resolution_clock::time_point start;
    long long count;
    ofstream *out = nullptr;
    bool getMinMax = !(minVal == nullptr || maxVal == nullptr);
    if (fpath != nullptr)
    {
        out = new ofstream(fpath);
    }

    double degreesPerSecond = 0, pastDegreesPerSecond = 0;
    while (samplesTaken < samples)
    {
        start = gyroClock.now();
        pastDegreesPerSecond = degreesPerSecond;
        degreesPerSecond = convertToDPS(gyro_z());
        accumulator += degreesPerSecond;
        theta += (degreesPerSecond + pastDegreesPerSecond) * 0.5 * secondsPerNSInterval;
        ++samplesTaken;
        count = (gyroClock.now() - start).count();
        if (count > nanosecondInterval)
        {
            std::cout << "fell behind!!" << std::endl;
        }
        if (fpath != nullptr)
        {
            (*out) << "dps is " << degreesPerSecond << " and theta is " << theta << endl;
        }
        if (getMinMax)
        {
            *minVal = min(*minVal, degreesPerSecond);
            *maxVal = max(*maxVal, degreesPerSecond);
        }

        std::this_thread::sleep_for(std::chrono::nanoseconds(nanosecondInterval - count));
    }
    if (fpath != nullptr)
    {
        out->close();
        delete out;
    }
}
void getGyroSamples(size_t samples)
{
    getGyroSamples(samples, nullptr);
}
void printGyroSamples(size_t samples, const char *fpath)
{
    getGyroSamples(samples, fpath);
}
void getGyroVals()
{
    std::chrono::high_resolution_clock::time_point start;
    long long count;
    double degreesPerSecond = 0, pastDegreesPerSecond = 0;
    while (run)
    {
        start = gyroClock.now();
        pastDegreesPerSecond = degreesPerSecond;
        degreesPerSecond = convertToDPS(gyro_z());
        theta += (degreesPerSecond + pastDegreesPerSecond) * 0.5 * secondsPerNSInterval;
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
    theta = 0;
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

void calibrateGyro(size_t samples)
{
    cout << "calibrating gyro..." << endl;
    cout << "with current rate of 1 sample every " << secondsPerNSInterval << " seconds, this will take "
         << samples * secondsPerNSInterval << " seconds." << endl;
    accumulator = 0;
    gyroSensitivity = get_gyro_sensitivity();
    getGyroSamples(samples / 2);
    bias_z = accumulator / samplesTaken;

    double minVal = numeric_limits<double>::max(), maxVal = numeric_limits<double>::min();
    getGyroSamples(samples / 2, nullptr, &minVal, &maxVal);
    minStationaryVal = minVal;
    maxStationaryVal = maxVal;

    std::cout << "gyro calibrated in z direction with bias " << bias_z << endl
              << " and min, max stationary boundary of " << minVal << ", " << maxVal << endl;
}

double getTheta()
{
    return theta;
}