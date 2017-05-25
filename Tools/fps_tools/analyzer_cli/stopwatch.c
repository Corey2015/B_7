#include "common.h"
#include "stopwatch.h"


#if defined(__WINDOWS__)
    #include <winsock.h>
    #if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
        #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
    #else
        #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
    #endif
#elif defined(__LINUX__)
    #include <sys/time.h>
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Calculate Elapsed Time
//

static void
stopwatch_get_current_time(struct timeval *time_value)
{
#if defined(__WINDOWS__)
    FILETIME       file_time;
    ULARGE_INTEGER curr_time;

    if (time_value != NULL) {
        GetSystemTimeAsFileTime(&file_time);

        curr_time.HighPart  = file_time.dwHighDateTime;
        curr_time.LowPart   = file_time.dwLowDateTime;
        curr_time.QuadPart /= 10;
        curr_time.QuadPart -= DELTA_EPOCH_IN_MICROSECS;

        time_value->tv_sec  = (long) curr_time.QuadPart / 1000000UL;
        time_value->tv_usec = (long) curr_time.QuadPart % 1000000UL;
    }
#elif defined(__LINUX__)
    gettimeofday(time_value, NULL);
#endif
}

void
stopwatch_start(stopwatch_t *record)
{
    stopwatch_get_current_time(&(record->start));
}

void
stopwatch_stop(stopwatch_t *record)
{
    stopwatch_get_current_time(&(record->stop));
}

double
stopwatch_get_elapsed(stopwatch_t *record)
{
    double elapsed;

    elapsed = (double) (record->stop.tv_sec  * (1000 * 1000) + record->stop.tv_usec) -
              (double) (record->start.tv_sec * (1000 * 1000) + record->start.tv_usec);

    return elapsed / 1000;
}
