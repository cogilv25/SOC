#ifndef COMPUTABILLITY_TIMING_H
#define COMPUTABILLITY_TIMING_H

#ifdef _WIN32
#include <Windows.h>
#endif

typedef enum
{
	SECOND = 1,
	MILLI = (int)1e+03,
	MICRO = (int)1e+06,
	NANO =  (int)1e+09
} TIMER_RESOLUTION;

void setResolution(TIMER_RESOLUTION);
void initHFT();
double hFTNow();

#endif