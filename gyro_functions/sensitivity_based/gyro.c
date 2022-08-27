#include <kipr/wombat.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "gyro_accumulator.h"
#define abs(a) (a >= 0 ? a : -a)
void turn_degrees(int degrees)
{

    setupGetGyroVals();
    startGetGyroVals();
    printf("driving");
    create_drive_direct(-100, 100);
    double theta = 0;
    while (abs((theta = getTheta())) < degrees)
    {
        printf("theta is %lf\n", theta);
        msleep(1);
    }
    create_drive_direct(0, 0);
    create_disconnect();
}
int main(int argnum, const char *args[])
{
    printf("argnum is %i\n", argnum);
    for (int i = 0; i < argnum; ++i)
    {
        printf("args of %i is ", i);
        printf(args[i]);
        printf("\n");
    }
    calibrateGyro(200);
    if (argnum == 2)
    {
        while (!create_connect_once())
        {
            printf("connecting\n");
            msleep(10);
        }
        create_full();
        turn_degrees(strtol(args[1], NULL, 10));
        create_disconnect();
    }
    if (argnum == 4)
    {
        printf("printing %i gyro samples to ", strtol(args[2], NULL, 10));
        printf(args[3]);
        printf("\n");
        printGyroSamples(strtol(args[2], NULL, 10), args[3]);
    }
    return 0;
}