#ifdef __cplusplus
#include <cstddef>
extern "C"
{
#else
#include <stdbool.h>
typedef unsigned long size_t;
#endif

    bool setupGetGyroVals();
    bool startGetGyroVals();
    bool cleanupGetGyroVals();

    void calibAccel(size_t samples);

    void printAccelSamples(const char *fpath, size_t samples);
    double getPositionX();
    double getPositionY();
#ifdef __cplusplus
}
#endif