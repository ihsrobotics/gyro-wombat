#ifdef __cplusplus
extern "C" {
#endif
double getAccumulator();
bool setBias(double _bias);
bool setupGetGyroVals();
bool cleanupGetGyroVals();
bool startGetGyroVals();
#ifdef __cplusplus
}
#endif