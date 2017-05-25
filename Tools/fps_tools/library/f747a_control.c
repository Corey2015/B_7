#include "common.h"
#include "fps.h"
#include "debug.h"
#include "fps_control.h"
#include "fps_calibration.h"
#include "f747a_register.h"


////////////////////////////////////////////////////////////////////////////////
//
// Initialize Sensor
//

int
f747a_init_sensor(fps_handle_t *handle)
{
    int     status = 0;
    uint8_t addr[5];
    uint8_t data[5];
	int     i;

    // Setup appropriate sensor speed
    status = fps_set_sensor_speed(handle, 8 * 1000 * 1000);
    if (status < 0) {
        return status;
    }

    // Reset sensor
    FPS_RESET_SENSOR(handle, 20.0 * 1000);

    // Disable and clear interrupts
    FPS_DISABLE_AND_CLEAR_INTERRUPT(handle, FPS_ALL_EVENTS);

    // Enter power down mode
    status = fps_switch_sensor_mode(handle, FPS_POWER_DOWN_MODE, NULL);
    if (status < 0) {
        return status;
    }

    // Turn off CP and LDO first
    status = fps_single_write(handle, FPS_REG_CPR_CTL_0, 0x00);
    if (status < 0) {
        return status;
    }
    fps_sleep(10.0 * 1000);

    // Specify the power up sequence in different configurations
    switch (handle->power_config) {
        case FPS_POWER_CONFIG_1V8_3V3 :
            data[0] = 0x08;
            data[1] = 0x04;
            data[2] = 0x0F;

			for (i = 0; i < 3; i++) {
                status = fps_single_write(handle, FPS_REG_CPR_CTL_0, data[i]);
                if (status < 0) {
                    return status;
                }
                fps_sleep(10.0 * 1000);
			}
            break;

        // TODO: Other power config. may be added later...
        default :
            return status = -1;
    }

    // Enter image mode
    status = fps_switch_sensor_mode(handle, FPS_IMAGE_MODE, NULL);
    if (status < 0) {
        return status;
    }
    fps_sleep(10.0 * 1000);

    // Settings for sensing enhancement
    addr[0] = FPS_REG_GBL_CTL;
    addr[1] = FPS_REG_MISC_PWR_CTL_1;
    addr[2] = FPS_REG_ANA_I_SET_0;
    addr[3] = FPS_REG_ANA_I_SET_1;
    addr[4] = FPS_REG_SET_ANA_0;

    data[0] = 0x00;
    data[1] = 0x20;
    data[2] = 0xFC;
    data[3] = 0x03;
    data[4] = 0xFF;

    status = fps_multiple_write(handle, addr, data, 5);
    if (status < 0) {
        return status;
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Switch Mode
//

int
f747a_set_sensor_mode(fps_handle_t *handle,
                      int          mode)
{
    int     status = 0;
    uint8_t addr[2];
    uint8_t data[2];

    addr[0] = FPS_REG_PWR_CTL_0;
    addr[1] = FPS_REG_GBL_CTL;

    data[0] = 0x00;
    data[1] = 0x00;

    status = fps_multiple_read(handle, addr, data, 2);
    if (status < 0) {
        return status;
    }

    // Enter Power Down mode first
    data[0]  = (FPS_PWRDWN_ALL & ~FPS_PWRDWN_BGR);
    data[1] &= ~FPS_ENABLE_DETECT;

    status = fps_multiple_write(handle, addr, data, 2);
    if (status < 0) {
        return status;
    }
    fps_sleep(10.0 * 1000);

    // Image mode
    if (mode == FPS_IMAGE_MODE) {
        data[0]  = (FPS_PWRDWN_DET | FPS_PWRDWN_OSC);
        data[1] &= ~FPS_ENABLE_DETECT;

        status = fps_multiple_write(handle, addr, data, 2);
        if (status < 0) {
            return status;
        }
        fps_sleep(10.0 * 1000);
    }

    // Detect mode
    if (mode == FPS_DETECT_MODE) {
        data[1] |= FPS_ENABLE_DETECT;

        status = fps_multiple_write(handle, addr, data, 2);
        if (status < 0) {
            return status;
        }
        fps_sleep(10.0 * 1000);
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Image Calibration
//

int
f747a_image_calibration(fps_handle_t *handle,
                        int          img_width,
                        int          img_height,
                        int          frms_to_avg)
{
    // Pre-defined parameters
    const int upper_bound      = 240;
    const int lower_bound      = 10;
    const int cds_offset_upper = FPS_MAX_CDS_OFFSET_1 - 0x80;
    const int cds_offset_lower = FPS_MIN_CDS_OFFSET_1 + 0x00;
    const int cds_offset_step  = 1;
    const int pga_gain_upper   = FPS_MAX_PGA_GAIN_1 - 0x02;
    const int pga_gain_lower   = FPS_MIN_PGA_GAIN_1 + 0x05;
    const int pga_gain_step    = 1;

    int            status = 0;
    int            sensor_width;
    int            sensor_height;
    int            col_begin;
    int            col_end;
    int            row_begin;
    int            row_end;
    int            top_row;
    int            middle_row;
    int            bottom_row;
    int            scan_col_begin;
    int            scan_col_end;
    int            scan_width;
    uint8_t        addr[3];
    uint8_t        data[3];
    int            cds_offset;
    int            pga_gain;
    int            too_bright;
    int            too_dark;
    fps_cal_info_t info;
    int            c;

    sensor_width  = handle->sensor_width;
    sensor_height = handle->sensor_height;

    col_begin = (sensor_width - img_width) / 2;
    col_end   = col_begin + img_width - 1;
    row_begin = (sensor_height - img_height) / 2;
    row_end   = row_begin + img_height - 1;
    
    top_row    = row_begin  + 4;
    middle_row = img_height / 2;
    bottom_row = row_end    - 4;

    scan_col_begin = col_begin + 8;
    scan_col_end   = col_end   - 8; 
    scan_width     = scan_col_end - scan_col_begin + 1;

    if (scan_width < 0) {
        return -1;
    }

    // Disable and clear all interrupts
    FPS_DISABLE_AND_CLEAR_INTERRUPT(handle, FPS_ALL_EVENTS);

    // Set image window
    status = fps_set_sensing_area(handle, FPS_IMAGE_MODE,
                                  col_begin, col_end,
                                  row_begin, row_end);
    if (status < 0) {
        return status;
    }

    // Initial conditions
    addr[0] = FPS_REG_IMG_CDS_CTL_0;
    addr[1] = FPS_REG_IMG_CDS_CTL_1;
    addr[2] = FPS_REG_IMG_PGA1_CTL;

    status = fps_multiple_read(handle, addr, data, 3);
    if (status < 0) {
        return status;
    }

    cds_offset = cds_offset_upper;
    pga_gain   = pga_gain_upper;

    while (1) {
        LOG_DEBUG("CDS Offset = 0x%03X, PGA Gain = 0x%02X\n",
                  cds_offset, pga_gain);

        data[0] = ((uint8_t) ((cds_offset & 0x100) >> 1)) | (data[0] & 0x7F);
        data[1] = ((uint8_t) ((cds_offset & 0x0FF) >> 0));
        data[2] = ((uint8_t) ((pga_gain   & 0x0F ) >> 0)) | (data[2] & 0xF0);

        status = fps_multiple_write(handle, addr, data, 3);
        if (status < 0) {
            return status;
        }

        // Get an image
        status = fps_get_averaged_image(handle,
                                        img_width,
                                        img_height,
                                        frms_to_avg,
                                        handle->bkgnd_img,
                                        &handle->bkgnd_avg,
                                        &handle->bkgnd_var,
                                        &handle->bkgnd_noise);
        if (status < 0) {
            return status;
        }

        if (handle->image_calibration_callback != NULL) {
            info.img_buf      = handle->bkgnd_img;
            info.img_avg      = handle->bkgnd_avg;
            info.img_var      = handle->bkgnd_var;
            info.img_noise    = handle->bkgnd_noise; 
            info.cds_offset_1 = cds_offset;
            info.pga_gain_1   = pga_gain;

            status = handle->image_calibration_callback(handle, &info);
            if (status < 0) {
                return status;
            }
        }

        too_bright = FALSE;
        too_dark   = FALSE;

        for (c = scan_col_begin; c <= scan_col_end; c++) {
            if (handle->bkgnd_img[top_row    * img_width + c] > upper_bound) {
                too_bright = TRUE;
                break;
            }

            if (handle->bkgnd_img[middle_row * img_width + c] < lower_bound) {
                too_dark = TRUE;
                break;
            }

            if (handle->bkgnd_img[bottom_row * img_width + c] > upper_bound) {
                too_bright = TRUE;
                break;
            }
        }

        LOG_DEBUG("%s\n", ((too_bright == TRUE) ? "Too Bright" :
                           (too_dark   == TRUE) ? "Too Dark"   : "OK"));

        if (too_bright == TRUE) {
            if (cds_offset < cds_offset_upper) {
                cds_offset += cds_offset_step;
                continue;
            }

            if (pga_gain > pga_gain_lower) {
                pga_gain   -= pga_gain_step;
                cds_offset  = cds_offset_upper;
                continue;
            }

            LOG_ERROR("Too bright but no more settings!\n");
            break;
        }

        if (too_dark == TRUE) {
            if (cds_offset > cds_offset_lower) {
                cds_offset -= cds_offset_step;
                continue;
            }

            if (pga_gain < pga_gain_upper) {
                pga_gain   += pga_gain_step;
                cds_offset  = cds_offset_upper;
                continue;
            }

            LOG_ERROR("Too dark but no more settings!\n");
            break;
        }

        break;
    }

    if ((too_bright == FALSE) && (too_dark == FALSE)) {
        return 0;
    } else {
        return -1;
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Detect Calibration
//

int
f747a_detect_calibration(fps_handle_t *handle,
                         int          det_width,
                         int          det_height,
                         int          frms_to_susp)
{

    // Pre-defined parameters
    const int    retry_limit      = 5;
    const int    detect_th_upper  = FPS_MAX_DETECT_TH;
    const int    detect_th_lower  = FPS_MIN_DETECT_TH;
    const int    cds_offset_upper = FPS_MAX_CDS_OFFSET_1;
    const int    cds_offset_lower = FPS_MIN_CDS_OFFSET_1;
    const double sleep_mul        = 2.0;

    int     status = 0;
    uint8_t addr[3];
    uint8_t data[3];
    int     pga_gain_init;
    int     cds_offset_init;
    int     cds_offset;
    int     detect_th;
    int     img_col_begin;
    int     img_col_end;
    int     img_row_begin;
    int     img_row_end;
    int     img_width;
    int     img_height;
    int     det_col_begin;
    int     det_col_end;
    int     det_row_begin;
    int     det_row_end;
    int     scan_width;
    int     scan_height;
    double  det_avg;
    double  det_var;
    double  det_noise;
    double  extra_cds_offset;
    double  sleep_us;
    int     retry_cnt;

    // 1. Copy Image Mode calibrated settings to Detect Mode
    // ------------------------------------------------------------------------

    FPS_DISABLE_AND_CLEAR_INTERRUPT(handle, FPS_ALL_EVENTS);

    addr[0] = FPS_REG_IMG_CDS_CTL_0;
    addr[1] = FPS_REG_IMG_CDS_CTL_1;
    addr[2] = FPS_REG_IMG_PGA1_CTL;

    status = fps_multiple_read(handle, addr, data, 3);
    if (status < 0) {
        return status;
    }

    addr[0] = FPS_REG_DET_CDS_CTL_0;
    addr[1] = FPS_REG_DET_CDS_CTL_1;
    addr[2] = FPS_REG_DET_PGA1_CTL;

    status = fps_multiple_write(handle, addr, data, 3);
    if (status < 0) {
        return status;
    }

    cds_offset_init = ((((int) data[0]) & 0x0080) << 1) | (((int) data[1]) & 0x00FF);
    pga_gain_init   = ((int) data[2]) & 0x0F;


    // 2. Set suspend interval
    // ------------------------------------------------------------------------

    // Set suspend interval
    status = fps_set_suspend_frames(handle, frms_to_susp);
    if (status < 0) {
        return status;
    }

    // Calculate corresponding sleep time
    sleep_us  = fps_calculate_suspend_time((det_width * det_height), frms_to_susp);
    sleep_us *= sleep_mul;


    // 3. Search the best detect window
    // -------------------------------------------------------------------------

    // Get current specified image window
    status = fps_get_sensing_area(handle, FPS_IMAGE_MODE,
                                  &img_col_begin, &img_col_end,
                                  &img_row_begin, &img_row_end);
    if (status < 0) {
        return status;
    }

    img_width  = img_col_end - img_col_begin + 1;
    img_height = img_row_end - img_row_begin + 1;

    scan_width  = det_width;
    scan_height = img_height - 40;

    // Find the best detect window
    status = fps_search_detect_window(handle,
		                              img_width, img_height,
                                      det_width, det_height,
                                      scan_width, scan_height,
                                      8,
                                      &det_col_begin, &det_col_end,
                                      &det_row_begin, &det_row_end,
                                      &det_avg, &det_var, &det_noise);
    if (status < 0) {
        return status;
    }

    // Set detect window
    status = fps_set_sensing_area(handle, FPS_DETECT_MODE,
                                  det_col_begin, det_col_end,
                                  det_row_begin, det_row_end);
    if (status < 0) {
        return status;
    }

    // Calculate the extra CDS Offset should be added
    extra_cds_offset = (int) ((det_avg - 35.0) / 10.0);
    LOG_DEBUG("Extra CDS Offset = %0.3f\n", extra_cds_offset);


    // 4. Search Detect Threshold
    // -------------------------------------------------------------------------

    // Search Detect Threshold using binary search
    for (retry_cnt = 0; retry_cnt < retry_limit; retry_cnt++) {

        status = fps_search_detect_threshold(handle,
                                             FPS_SEARCH_ONE_TRIGGER,
                                             detect_th_upper,
                                             detect_th_lower,
                                             sleep_us,
                                             1,
                                             &detect_th);
        if (status == -2) {
            continue;
        }

        if (status < 0) {
            return status;
        }

        break;
    }

    if ((status == -2) && (retry_cnt == retry_limit)) {
        LOG_DEBUG("Retry time out! Exit...\n");
        return -1;
    }

    // Fine tune Detect Threshold
    for (; retry_cnt < retry_limit; retry_cnt++) {

        status = fps_finetune_detect_threshold_downward(handle,
                                                        detect_th_upper,
                                                        detect_th_lower,
                                                        1,
                                                        sleep_us,
                                                        1,
                                                        &detect_th);
        if (status == -2) {
            continue;
        }

        if (status < 0) {
            return status;
        }

        break;
    }

    if ((status == -2) && (retry_cnt == retry_limit)) {
        LOG_DEBUG("Retry time out! Exit...\n");
        return -1;
    }

    for (; retry_cnt < retry_limit; retry_cnt++) {

        status = fps_finetune_detect_threshold_upward(handle,
                                                      detect_th_upper,
                                                      detect_th_lower,
                                                      1,
                                                      sleep_us,
                                                      1,
                                                      &detect_th);
        if (status == -2) {
            continue;
        }

        if (status < 0) {
            return status;
        }

        break;
    }

    if ((status == -2) && (retry_cnt == retry_limit)) {
        LOG_DEBUG("Retry time out! Exit...\n");
        return -1;
    }

    // 5. Search CDS Offset
    // -------------------------------------------------------------------------

    // Search CDS Offset using binary search
    for (; retry_cnt < retry_limit; retry_cnt++) {

        status = fps_search_detect_cds_offset(handle,
                                              FPS_SEARCH_ONE_TRIGGER,
                                              cds_offset_upper,
                                              cds_offset_lower,
                                              sleep_us,
                                              4,
                                              &cds_offset);
        if (status == -2) {
            continue;
        }

        if (status < 0) {
            return status;
        }

        break;
    }

    if ((status == -2) && (retry_cnt == retry_limit)) {
        LOG_DEBUG("Retry time out! Exit...\n");
        return -1;
    }

    // Fine tune CDS Offset
    for (; retry_cnt < retry_limit; retry_cnt++) {

        status = fps_finetune_detect_cds_offset_downward(handle,
                                                         cds_offset_upper,
                                                         cds_offset_lower,
                                                         1,
                                                         sleep_us,
                                                         4,
                                                         &cds_offset);
        if (status == -2) {
            continue;
        }

        if (status < 0) {
            return status;
        }

        break;
    }

    if ((status == -2) && (retry_cnt == retry_limit)) {
        LOG_DEBUG("Retry time out! Exit...\n");
        return -1;
    }

    for (; retry_cnt < retry_limit; retry_cnt++) {

        status = fps_finetune_detect_cds_offset_upward(handle,
                                                       cds_offset_upper,
                                                       cds_offset_lower,
                                                       1,
                                                       sleep_us,
                                                       4,
                                                       &cds_offset);
        if (status == -2) {
            continue;
        }

        if (status < 0) {
            return status;
        }

        break;
    }

    if ((status == -2) && (retry_cnt == retry_limit)) {
        LOG_DEBUG("Retry time out! Exit...\n");
        return -1;
    }

    return status;
}
