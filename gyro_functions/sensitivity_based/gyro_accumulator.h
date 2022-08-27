#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stdbool.h>
#endif
    void getGyroVals();
    bool setupGetGyroVals();
    bool cleanupGetGyroVals();
    bool startGetGyroVals();
    void calibrateGyro(size_t samples);

    double getTheta();
    void printGyroSamples(size_t samples, const char *fpath);
    void getGyroSamples(size_t samples);
#ifdef __cplusplus
}
#endif