#ifndef __common_h__
#define __common_h__


///////////////////////////////////////////////////////////////////////////////
//
// Environment
//

#if !defined(__WINDOWS__)
    #if (defined(_WINDOWS) || defined(WINDOWS))
        #define __WINDOWS__
    #endif
#endif

#if !defined(__LINUX__)
    #if (defined(__linux) || defined(__linux__))
        #define __LINUX__
    #endif
#endif

#if !defined(__DEBUG__)
    #if (defined(_DEBUG) || defined(DEBUG))
        #define __DEBUG__
    #endif
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Include Dependencies
//

#if defined(__WINDOWS__)
    #include <windows.h>
#endif
#if defined(__cplusplus)
    #include <string>
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Data Types
//

// NOTE: In VC6, data types such as uint8_t is not defined.
#if defined(__WINDOWS__)
    #include "stdint.h"
    #include "inttypes.h"
#elif defined(__LINUX__)
    #include <stdint.h>
    #include <inttypes.h>
#endif

typedef char               i08_t;
typedef short int          i16_t;
typedef int                i32_t;
typedef unsigned char      u08_t;
typedef unsigned short int u16_t;
typedef unsigned int       u32_t;
typedef signed char        s08_t;
typedef signed short int   s16_t;
typedef signed int         s32_t;
typedef float              f32_t;
typedef double             f64_t;
typedef int                bit_t;
typedef char               char_t;

#if defined(__cplusplus)
    typedef std::string    str_t;
    typedef bool           bool_t;
#else
    typedef int            bool_t;
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Constants
//

// Null pointer
#if !defined(NULL)
    #if defined(__cplusplus)
        #define NULL (0)
    #else
        #define NULL ((void *) 0)
    #endif
#endif

// Maximum and minimum values for strings
#define MAX_FNAME_LENGTH  (4096)
#define MAX_STRING_LENGTH (256)


////////////////////////////////////////////////////////////////////////////////
//
// Basic Conditions
//

// True or False
#if !defined(TRUE)
    #define TRUE (1 == 1)
#endif
#if !defined(FALSE)
    #define FALSE (!TRUE)
#endif

// Pass or Fail
#if !defined(PASS)
    #define PASS (TRUE)
#endif
#if !defined(FAIL)
    #define FAIL (!PASS)
#endif

// High or Low
#if !defined(HIGH)
    #define HIGH (TRUE)
#endif
#if !defined(LOW)
    #define LOW (!HIGH)
#endif

// String match or mismatch
#define STRING_MATCHED    (0)
#define STRING_MISMATCHED (!STRING_MATCHED)


////////////////////////////////////////////////////////////////////////////////
//
// Function Argument Attributes
//

#if !defined(__INPUT)
    #define __INPUT const
#endif

#if !defined(__OUTPUT)
    #define __OUTPUT
#endif

#if !defined(__INOUT)
    #define __INOUT
#endif

#if defined(__cplusplus)
	#define INLINE inline
#else
	#if (defined(__WINDOWS__) && (_MSC_VER == 1200))
		#define __INLINE __forceinline
	#else
		#define __INLINE __attribute__((always_inline))
	#endif
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Basic Utilities
//

#if !defined(DUMMY_VAR)
    #if defined(__WINDOWS__)
        #define DUMMY_VAR(_x_) UNREFERENCED_PARAMETER(_x_)
    #else
        #define DUMMY_VAR(_x_) (_x_)
    #endif
#endif

#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(_x_) (sizeof(_x_) / sizeof((_x_)[0]))
#endif

#if !defined(BIT)
    #define BIT(_x_) (1 << (_x_))
#endif

#if !defined(MAX)
    #define MAX(_a_, _b_) (((_a_) > (_b_)) ? (_a_) : (_b_))
#endif

#if !defined(MIN)
    #define MIN(_a_, _b_) (((_a_) < (_b_)) ? (_a_) : (_b_))
#endif

#if !defined(CONSTRAINT)
    #define CONSTRAINT(_u_, _x_, _l_) \
        (((_x_) > (_u_)) ? (_u_) : ((_x_) < (_l_)) ? (_l_) : (_x_))
#endif

#if !defined(SQUARE)
    #define SQUARE(_x_) ((_x_) * (_x_))
#endif

#if !defined(__FUNC__)
    #if (defined(__WINDOWS__) && (_MSC_VER == 1200)) // For Visual Studio 6
        #define __FUNC__ TEXT(" ")
    #else
        #define __FUNC__ __func__
    #endif
#endif


#endif // __common_h__
