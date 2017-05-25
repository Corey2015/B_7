#ifndef __f747b_control_h__
#define __f747b_control_h__


#include "fps.h"


#if defined(__cplusplus)
extern "C" {
#endif

    
int f747b_init_sensor(fps_handle_t *handle);

int f747b_set_sensor_mode(fps_handle_t *handle,
                          int          mode);

int f747b_image_calibration(fps_handle_t *handle,
                            int          img_width,
                            int          img_height,
                            int          frms_to_avg);

int f747b_detect_calibration(fps_handle_t *handle,
                             int          det_width,
                             int          det_height,
                             int          frms_to_susp);


#if defined(__cplusplus)
}
#endif


#endif // __f747b_control_h__
