#ifndef __debug_h__
#define __debug_h__


#include <stdio.h>
#include "common.h"


#if defined(__cplusplus)
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Debug Level
//

enum {
	LOG_LEVEL_ERROR  = 0,
	LOG_LEVEL_WARN   = 1,
	LOG_LEVEL_INFO   = 2,
	LOG_LEVEL_DEBUG  = 3,
	LOG_LEVEL_DETAIL = 4,
};


////////////////////////////////////////////////////////////////////////////////
//
// Debug Messages
//

#if (defined(__WINDOWS__) && (_MSC_VER == 1200))
    #define LOG_ERROR  printf
    #define LOG_WARN   printf
    #define LOG_INFO   printf
    #define LOG_DEBUG  printf
    #define LOG_DETAIL printf
#else
    #define LOG_ERROR(...)  do { (void) print_error (__FILE__, __LINE__, __VA_ARGS__); } while(0)
    #define LOG_WARN(...)   do { (void) print_warn  (__FILE__, __LINE__, __VA_ARGS__); } while(0)
    #define LOG_INFO(...)   do { (void) print_info  (__FILE__, __LINE__, __VA_ARGS__); } while(0)
    #define LOG_DEBUG(...)  do { (void) print_debug (__FILE__, __LINE__, __VA_ARGS__); } while(0)
    #define LOG_DETAIL(...) do { (void) print_detail(__FILE__, __LINE__, __VA_ARGS__); } while(0)
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//

int print_error(char *file,
                int  line,
                char *format, ...);

int print_warn(char *file,
               int  line,
               char *format, ...);

int print_info(char *file,
               int  line,
               char *format, ...);

int print_debug(char *file,
                int  line,
                char *format, ...);

int print_detail(char *file,
                 int  line,
                 char *format, ...);

int set_debug_level(int level);
int get_debug_level(int *level);


#if defined(__cplusplus)
}
#endif


#endif // __debug_h__
