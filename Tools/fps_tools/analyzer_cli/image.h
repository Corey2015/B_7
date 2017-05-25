#ifndef __image_h__
#define __image_h__


#include "common.h"


#if defined(__cplusplus)
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//

int save_bmp(char    *path,
             uint8_t *img,
             size_t  size);


#if defined(__cplusplus)
}
#endif


#endif // __image_h__
