#include "timing.h"

#ifndef _WIN32
#include <time.h>
#endif

static double timerFQ;
static TIMER_RESOLUTION resolution = SECOND;

void initHFT()
{
#ifdef _WIN32
	LARGE_INTEGER t;
	QueryPerformanceFrequency(&t);
	timerFQ = t.QuadPart;
#else
	timerFQ = CLOCKS_PER_SEC;
#endif
}

// I removed resolution since we are just going to be using
//    SECOND every time anyway..
double hFTNow()
{
	double time;
#ifdef _WIN32
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	time = t.QuadPart * resolution;
#else
	time =  clock() * resolution;
#endif
	return time / timerFQ;
}

void setResolution(TIMER_RESOLUTION newRes)
{
	resolution = newRes;
}