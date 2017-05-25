#ifndef __fps_h__
#define __fps_h__


#if defined(__cplusplus)
extern "C" {
#endif


typedef struct __fps_handle fps_handle_t;

typedef struct __fps_cal_info fps_cal_info_t;

typedef int (*fps_cal_callback_t) (fps_handle_t   *handle,
                                   fps_cal_info_t *info);


////////////////////////////////////////////////////////////////////////////////
//
// Sensor Device Handle
//

struct __fps_handle {
    int           fd;
    int           chip_id;
    int           sensor_width;
    int           sensor_height;
    int           latency;
    int           power_config;
    unsigned char *bkgnd_img;
    double        bkgnd_avg;
    double        bkgnd_var;
    double        bkgnd_noise;

    int (*init_sensor_method) (fps_handle_t *handle);

    int (*set_sensor_mode_method) (fps_handle_t *handle,
                                   int          mode);

    int (*image_calibration_method) (fps_handle_t *handle,
                                     int          img_width,
                                     int          img_height,
                                     int          frms_to_avg);

    int (*image_calibration_callback) (fps_handle_t   *handle,
                                       fps_cal_info_t *info);

    int (*detect_calibration_method) (fps_handle_t *handle,
                                      int          det_width,
                                      int          det_height,
                                      int          frms_to_susp);

    int (*detect_calibration_callback) (fps_handle_t   *handle,
                                        fps_cal_info_t *info);

    int (*scan_detect_method) (fps_handle_t *handle,
                               double       sleep_us);
};


////////////////////////////////////////////////////////////////////////////////
//
// Sensor Calibration Information
//

struct __fps_cal_info {
    unsigned char *img_buf;
    double        img_avg;
    double        img_var;
    double        img_noise;
    int           cds_offset_0;
    int           cds_offset_1;
    int           pga_gain_0;
    int           pga_gain_1;
    int           detect_th;
};


////////////////////////////////////////////////////////////////////////////////
//
// Sensor Open/Close/Reset/Init
//

extern fps_handle_t *fps_open_sensor(char *file);

extern int fps_close_sensor(fps_handle_t **handle);
                                 
extern int fps_init_sensor(fps_handle_t *handle);

extern int fps_reset_sensor(fps_handle_t *handle,
                            int          state);
                                 
extern fps_handle_t* fps_attach_sensor(int fd);

extern int fps_detach_sensor(fps_handle_t **handle);


////////////////////////////////////////////////////////////////////////////////
//
// Mode Switch
//

enum {
    FPS_IMAGE_MODE      = 0,
    FPS_DETECT_MODE     = 1,
    FPS_POWER_DOWN_MODE = 2,
};

extern int fps_set_sensor_mode(fps_handle_t *handle,
                               int          mode);

extern int fps_get_sensor_mode(fps_handle_t *handle,
                               int          *mode);

extern int fps_switch_sensor_mode(fps_handle_t *handle,
                                  int          mode_new,
                                  int          *mode_old);


////////////////////////////////////////////////////////////////////////////////
//
// Image Mode Operations
//

extern int fps_image_calibration(fps_handle_t *handle,
                                 int          img_width,
                                 int          img_height,
                                 int          frms_to_avg);

extern int fps_set_image_calibration_callback(fps_handle_t       *handle,
                                              fps_cal_callback_t callback);

extern int fps_get_averaged_image(fps_handle_t  *handle,
                                  int           img_width,
                                  int           img_height,
                                  int           frms_to_avg,
                                  unsigned char *img_buf,
                                  double        *img_avg,
                                  double        *img_var,
                                  double        *img_noise);

extern int fps_get_finger_image(fps_handle_t  *handle,
                                int           img_width,
                                int           img_height,
                                int           frms_to_avg,
                                unsigned char *img_buf,
                                double        *img_dr,
                                double        *img_var,
                                double        *img_noise);

extern int fps_get_background_average(fps_handle_t *handle,
                                      double       *img_avg);

extern int fps_get_background_variance(fps_handle_t *handle,
                                       double       *img_var);

extern int fps_get_background_noise(fps_handle_t *handle,
                                    double       *img_noise);


////////////////////////////////////////////////////////////////////////////////
//
// Detect Mode Operations
//

extern int fps_detect_calibration(fps_handle_t *handle,
                                  int          det_width,
                                  int          det_height,
                                  int          frms_to_susp);

extern int fps_set_detect_calibration_callback(fps_handle_t       *handle,
                                               fps_cal_callback_t callback);

extern int fps_scan_detect_event(fps_handle_t *handle,
                                 double       sleep_us);


#if defined(__cplusplus)
}
#endif


#endif // __fps_h__
