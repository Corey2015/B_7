#ifndef __fps_register_h__
#define __fps_register_h__


#if defined(__F747A__)
    #include "f747a_register.h"
#elif defined(__F747B__)
    #include "f747b_register.h"
#else
    #error " [ ERROR ] Sensor type must be specified! "
#endif


#endif // __fps_register_h__
