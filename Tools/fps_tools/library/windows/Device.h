#ifndef __Device_h__
#define __Device_h__


#include "common.h"


#if defined(__cplusplus)
extern "C" {
#endif


struct Device {
	char Name[MAX_STRING_LENGTH];
	char Path[MAX_FNAME_LENGTH];
};


#if defined(__cplusplus)
}
#endif


#endif // __Device_h__
