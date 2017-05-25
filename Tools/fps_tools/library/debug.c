#include <stdarg.h>
#include <string.h>
#include "common.h"
#include "debug.h"


#if defined(__WINDOWS__)
    #define SNPRINTF  _snprintf
    #define VSNPRINTF _vsnprintf
    #if defined(__MFC__)
        #define DEBUG_OUTPUT(_str_) OutputDebugString(_str_)
    #else
        #define DEBUG_OUTPUT(_str_) printf(_str_)
    #endif
#else
    #define SNPRINTF  snprintf
    #define VSNPRINTF vsnprintf
    #define DEBUG_OUTPUT(_str_) printf(_str_)
#endif


#if defined(__DEBUG__)
int debug_level = LOG_LEVEL_DEBUG;
#else
int debug_level = LOG_LEVEL_ERROR;
#endif
static char last_char = '\n';

static int
print_message(int     level,
              char    *file,
              int     line,
              char    *format,
              va_list arg_ptr)
{
    if (format == NULL) {
        return -1;
    }

    if (level <= debug_level) {
        char text[MAX_STRING_LENGTH];
        int  num_of_chars = 0;

        if (last_char == '\n') {
            switch (level) {
                case LOG_LEVEL_ERROR  : num_of_chars = SNPRINTF(text, MAX_STRING_LENGTH, " [ ERROR ] "); break;
                case LOG_LEVEL_WARN   : num_of_chars = SNPRINTF(text, MAX_STRING_LENGTH, "  [ WARN ] "); break;
                case LOG_LEVEL_INFO   : num_of_chars = SNPRINTF(text, MAX_STRING_LENGTH, "  [ INFO ] "); break;
                case LOG_LEVEL_DEBUG  : num_of_chars = SNPRINTF(text, MAX_STRING_LENGTH, " [ DEBUG ] "); break;
                case LOG_LEVEL_DETAIL : num_of_chars = SNPRINTF(text, MAX_STRING_LENGTH, "[ DETAIL ] "); break;
                default : return -1;
            }

            if (debug_level >= LOG_LEVEL_DEBUG) {
                num_of_chars = SNPRINTF(&text[strlen(text)], (MAX_STRING_LENGTH - strlen(text)), "(%s:%d) ", file, line);
            }

            if (num_of_chars < 0) {
                return -1;
            }
        }

        num_of_chars = VSNPRINTF(&text[strlen(text)], (MAX_STRING_LENGTH - strlen(text)), format, arg_ptr);
        if (num_of_chars < 0) {
            return -1;
        }

        last_char = text[strlen(text) - 1];
        DEBUG_OUTPUT(text);
    }

    return 0;
}

int
print_error(char *file,
            int  line,
            char *format, ...)
{
    int     status = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    status = print_message(LOG_LEVEL_ERROR, file, line, format, arg_ptr);
    va_end(arg_ptr);
    
    return status;
}

int
print_warn(char *file,
           int  line,
           char *format, ...)
{
    int     status = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    status = print_message(LOG_LEVEL_WARN, file, line, format, arg_ptr);
    va_end(arg_ptr);
    
    return status;
}

int
print_info(char *file,
           int  line,
           char *format, ...)
{
    int     status = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    status = print_message(LOG_LEVEL_INFO, file, line, format, arg_ptr);
    va_end(arg_ptr);
    
    return status;
}

int
print_debug(char *file,
            int  line,
            char *format, ...)
{
    int     status = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    status = print_message(LOG_LEVEL_DEBUG, file, line, format, arg_ptr);
    va_end(arg_ptr);
    
    return status;
}

int
print_detail(char *file,
             int  line,
             char *format, ...)
{
    int     status = 0;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    status = print_message(LOG_LEVEL_DETAIL, file, line, format, arg_ptr);
    va_end(arg_ptr);
    
    return status;
}

int
set_debug_level(int level)
{
    if ((level < LOG_LEVEL_ERROR) || (level > LOG_LEVEL_DETAIL)) {
        return -1;
    }

    debug_level = level;
    return 0;
}

int
get_debug_level(int *level)
{
    *level = debug_level;
    return 0;
}
