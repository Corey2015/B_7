#ifndef __stopwatch_h__
#define __stopwatch_h__


#if defined(__WINDOWS__)
    #include <winsock.h>
#elif defined(__LINUX__)
    #include <sys/time.h>
#endif


#if defined(__cplusplus)
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Calculate Elapsed Time
//

typedef struct __stopwatch {
    struct timeval start;
    struct timeval stop;
}
stopwatch_t;

void stopwatch_start(stopwatch_t *record);

void stopwatch_stop(stopwatch_t *record);

double stopwatch_get_elapsed(stopwatch_t *record);


#if defined(__cplusplus)
}
#endif


#endif // __stopwatch_h__
