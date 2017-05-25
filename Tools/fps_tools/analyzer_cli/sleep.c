#if defined(__WINDOWS__)
    #include <winsock.h>
    #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
        #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
    #else
        #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
    #endif
#elif defined(__LINUX__)
    #include <unistd.h>
#endif

#include "debug.h"


////////////////////////////////////////////////////////////////////////////////
//
// Sleep in Microseconds
//

void
sleep_us(double usec)
{
#if defined(__WINDOWS__)
	__int64 elapsed_time;
	__int64 start_time;
	__int64 freq;
	__int64 wait_time;

    QueryPerformanceFrequency((LARGE_INTEGER *) &freq);

	wait_time = (__int64) (((double) freq) * usec / ((double) 1000000.0f) + 0.5);
	
    QueryPerformanceCounter((LARGE_INTEGER *) &start_time);

	elapsed_time = start_time;

	while ((elapsed_time - start_time) < wait_time) {
        QueryPerformanceCounter((LARGE_INTEGER *) &elapsed_time);
    }
#elif defined(__LINUX__)
    usleep((useconds_t) (usec + 0.5));
#endif
}


////////////////////////////////////////////////////////////////////////////////
//
// Sleep in Milliseconds
//

void
sleep_ms(double msec)
{
#if defined(__WINDOWS__)
    Sleep((DWORD) (msec + 0.5));
#elif defined(__LINUX__)
    usleep((useconds_t) (msec * 1000 + 0.5));
#endif
}

