#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#endif
    double getAccumulator();
    bool setBias(double _bias);
    double getBias();
    bool setupGetGyroVals();
    bool cleanupGetGyroVals();
    bool startGetGyroVals();
    void getGyroSamples(size_t samples);
    int getSamplesTaken();
    void calibrateGyro(size_t samples);
#ifdef __cplusplus
}
#endif