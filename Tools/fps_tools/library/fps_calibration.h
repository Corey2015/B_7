#ifndef __fps_calibration_h__
#define __fps_calibration_h__


#include "fps.h"


#if defined(__cplusplus)
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Image Calibration
//

int fps_image_calibration(fps_handle_t *handle,
                          int          img_width,
                          int          img_height,
                          int          frms_to_avg);

int fps_set_image_calibration_callback(fps_handle_t       *handle,
                                       fps_cal_callback_t callback);


////////////////////////////////////////////////////////////////////////////////
//
// Detect Calibration
//

int fps_detect_calibration(fps_handle_t *handle,
                           int          det_width,
                           int          det_height,
                           int          frms_to_susp);

int fps_set_detect_calibration_callback(fps_handle_t       *handle,
                                        fps_cal_callback_t callback);


////////////////////////////////////////////////////////////////////////////////
//
// Helpers
//

int fps_search_detect_window(fps_handle_t *handle,
                             int          img_width,
                             int          img_height,
                             int          det_width,
                             int          det_height,
                             int          scan_width,
                             int          scan_height,
                             int          frames,
                             int          *det_col_begin,
                             int          *det_col_end,
                             int          *det_row_begin,
                             int          *det_row_end,
                             double       *det_avg,
                             double       *det_var,
                             double       *det_noise);

enum {
    FPS_SEARCH_ONE_TRIGGER = 0,
    FPS_SEARCH_NO_TRIGGER  = 1,
};

int fps_search_detect_threshold(fps_handle_t *handle,
                                int          search_mode,
                                int          upper_bound,
                                int          lower_bound,
                                double       sleep_us,
                                int          scan_limit,
                                int          *detect_th);

int fps_finetune_detect_threshold_downward(fps_handle_t *handle,
                                           int          upper_bound,
                                           int          lower_bound,
                                           int          check_range,
                                           double       sleep_us,
                                           int          scan_limit,
                                           int          *detect_th);

int fps_finetune_detect_threshold_upward(fps_handle_t *handle,
                                         int          upper_bound,
                                         int          lower_bound,
                                         int          check_range,
                                         double       sleep_us,
                                         int          scan_limit,
                                         int          *detect_th);

int fps_search_detect_cds_offset(fps_handle_t *handle,
                                 int          search_mode,
                                 int          upper_bound,
                                 int          lower_bound,
                                 double       sleep_us,
                                 int          scan_limit,
                                 int          *cds_offset);

int fps_finetune_detect_cds_offset_downward(fps_handle_t *handle,
                                            int          upper_bound,
                                            int          lower_bound,
                                            int          check_range,
                                            double       sleep_us,
                                            int          scan_limit,
                                            int          *cds_offset);

int fps_finetune_detect_cds_offset_upward(fps_handle_t *handle,
                                          int          upper_bound,
                                          int          lower_bound,
                                          int          check_range,
                                          double       sleep_us,
                                          int          scan_limit,
                                          int          *cds_offset);


#if defined(__cplusplus)
}
#endif


#endif // __fps_calibration_h__
