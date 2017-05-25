#include <math.h>
#include "common.h"
#include "debug.h"
#include "fps.h"
#include "fps_control.h"
#include "fps_calibration.h"
#include "f747b_register.h"


////////////////////////////////////////////////////////////////////////////////
//
// Initialize Sensor
//

int
f747b_init_sensor(fps_handle_t *handle)
{
    const double init_sensor_delay_us = 50.0 * 1000;

    int     status = 0;
    uint8_t addr[6];
    uint8_t data[6];
	int     i;

    // Setup appropriate SPI speed
    status = fps_set_sensor_speed(handle, 8 * 1000 * 1000);
    if (status < 0) {
        return status;
    }

    // Reset sensor
    FPS_RESET_SENSOR(handle, 20.0 * 1000);

    // Disable and clear interrupts
    FPS_DISABLE_AND_CLEAR_INTERRUPT(handle, FPS_ALL_EVENTS);

    // Turn off CP and LDO1/2 first
    addr[0] = FPS_REG_CPR_CTL_0;
    addr[1] = FPS_REG_CPR_CTL_1;
    
    data[0] = 0x00;  // LDO1 off, CP off, LDO2 off
    data[1] = 0x00;

    status = fps_multiple_write(handle, addr, data, 2);
    if (status < 0) {
        return status;
    }
    fps_sleep(init_sensor_delay_us);

    // Specify the power up sequence in different configurations
    switch (handle->power_config) {
        case FPS_POWER_CONFIG_1V8_3V3 :
            data[0] = 0x10; data[1] = 0x2C;  // LDO1 off, CP off, LDO2 pull-down
            data[2] = 0x10; data[3] = 0x3C;  // LDO1 off, CP off, LDO2 on

			for (i = 0; i < 2; i++) {
                status = fps_multiple_write(handle, addr, &data[(i * 2)], 2);
                if (status < 0) {
                    return status;
                }
                fps_sleep(init_sensor_delay_us);
			}
            break;

        case FPS_POWER_CONFIG_1V8_2V8 :
        case FPS_POWER_CONFIG_1V8_ONLY :
            data[0] = 0x38; data[1] = 0x2C;  // LDO1 pull-up, CP off, LDO2 pull-up
            data[2] = 0xFC; data[3] = 0x2C;  // LDO1 on,      CP on,  LDO2 pull-up
            data[4] = 0xFC; data[5] = 0x3C;  // LDO1 on,      CP on,  LDO2 on

			for (i = 0; i < 3; i++) {
                status = fps_multiple_write(handle, addr, &data[(i * 2)], 2);
                if (status < 0) {
                    return status;
                }
                fps_sleep(init_sensor_delay_us);
			}
            break;

		default :
			return status = -1;
    }

    // Settings for sensing enhancement
    addr[0] = FPS_REG_IMG_PGA0_CTL;
    addr[1] = FPS_REG_ANA_I_SET_1;
    addr[2] = FPS_REG_SET_ANA_0;
    addr[3] = FPS_REG_MISC_PWR_CTL_1;

    data[0] = 0x20;
    data[1] = 0x03;
    data[2] = 0x0F;
    data[3] = 0x20;

    status = fps_multiple_write(handle, addr, data, 5);
    if (status < 0) {
        return status;
    }

    fps_sleep(init_sensor_delay_us);

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Sensor Mode
//

int
f747b_set_sensor_mode(fps_handle_t *handle,
                      int          mode)
{
    const double set_mode_delay_us = 20.0 * 1000;

    int     status = 0;
    uint8_t addr[9];
    uint8_t data[9];

    addr[0] = FPS_REG_CPR_CTL_1; addr[1] = DUMMY_DATA;
    addr[2] = FPS_REG_PWR_CTL_0; addr[3] = DUMMY_DATA;
    addr[4] = FPS_REG_GBL_CTL;   addr[5] = DUMMY_DATA;
    addr[6] = FPS_REG_PWR_CTL_0; addr[7] = DUMMY_DATA;
    addr[8] = FPS_REG_CPR_CTL_1;

    data[0] = 0x3C;              data[1] = DUMMY_DATA;
    data[2] = 0x00;              data[3] = DUMMY_DATA;
    data[4] = 0x00;              data[5] = DUMMY_DATA;
    data[6] = 0x00;              data[7] = DUMMY_DATA;
    data[8] = 0x2C;

    status = fps_single_read(handle, FPS_REG_GBL_CTL, &data[4]);

    // Go to Power Down Mode first
    data[4] &= ~(FPS_ENABLE_DETECT | FPS_ENABLE_PWRDWN);
    data[6]  = (FPS_PWRDWN_ALL & ~FPS_PWRDWN_BGR);

    status = fps_multiple_write(handle, &addr[4], &data[4], 5);
    if (status < 0) {
        return status;
    }

    fps_sleep(set_mode_delay_us);

    switch (mode) {
        case FPS_IMAGE_MODE :
            data[2]  = (FPS_PWRDWN_DET | FPS_PWRDWN_OSC);
            data[4] &= ~(FPS_ENABLE_DETECT | FPS_ENABLE_PWRDWN);

            status = fps_multiple_write(handle, &addr[0], &data[0], 5);
            if (status < 0) {
                return status;
            }

            fps_sleep(set_mode_delay_us);
            break;

        case FPS_DETECT_MODE :
            data[2]  = (FPS_PWRDWN_ALL & ~FPS_PWRDWN_BGR);
            data[4] |= (FPS_ENABLE_DETECT | FPS_ENABLE_PWRDWN);

            status = fps_multiple_write(handle, &addr[0], &data[0], 5);
            if (status < 0) {
                return status;
            }

            fps_sleep(set_mode_delay_us);
            break;

        default : break;
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Image Calibration
//

int
f747b_image_calibration(fps_handle_t *handle,
                        int          img_width,
                        int          img_height,
                        int          frms_to_avg)
{
    // Pre-defined parameters
    const int    upper_bound      = 240;
    const int    lower_bound      = 10;
    const double ratio            = 0.05;
    const int    cds_offset_upper = FPS_MAX_CDS_OFFSET_1 - 0x80;
    const int    cds_offset_lower = FPS_MIN_CDS_OFFSET_1 + 0x00;
    const int    cds_offset_step  = 1;
    const int    pga_gain_upper   = FPS_MAX_PGA_GAIN_1 - 0x02;
    const int    pga_gain_lower   = FPS_MIN_PGA_GAIN_1 + 0x05;
    const int    pga_gain_step    = 1;

    int            status = 0;
	int            sensor_width;
	int            sensor_height;
	int            col_begin;
	int            col_end;
	int            row_begin;
	int            row_end;
    int            scan_col_begin;
    int            scan_col_end;
    int            scan_row_begin;
    int            scan_row_end;
    int            scan_width;
    int            scan_height;
    size_t         scan_size;
    uint8_t        addr[3];
    uint8_t        data[3];
    int            cds_offset;
    int            pga_gain;
    int            brighter_pixels;
    int            darker_pixels;
    int            too_bright;
    int            too_dark;
    fps_cal_info_t info;
    int            r;
    int            c;

    sensor_width  = handle->sensor_width;
    sensor_height = handle->sensor_height;

    col_begin = (sensor_width - img_width) / 2;
    col_end   = col_begin + img_width - 1;
    row_begin = (sensor_height - img_height) / 2;
    row_end   = row_begin + img_height - 1;

    scan_col_begin = col_begin + (img_width  / 4);
    scan_col_end   = col_end   - (img_width  / 4);
    scan_row_begin = row_begin + (img_height / 4);
    scan_row_end   = row_end   - (img_height / 4);

    scan_width  = scan_col_end - scan_col_begin + 1;
    scan_height = scan_row_end - scan_row_begin + 1;
    scan_size   = scan_width * scan_height;

    if ((scan_width < 0) || (scan_height < 0)) {
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
        data[2] = ((uint8_t) ((pga_gain & 0x0F ) >> 0)) | (data[2] & 0xF0);

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

        // Do calibration
        brighter_pixels = 0;
        darker_pixels   = 0;

        for (r = scan_row_begin; r <= scan_row_end; r++) {
        for (c = scan_col_begin; c <= scan_col_end; c++) {
            if (handle->bkgnd_img[r * img_width + c] > upper_bound) {
                brighter_pixels++;
            }
            if (handle->bkgnd_img[r * img_width + c] < lower_bound) {
                darker_pixels++;
            }
        }}

        too_bright = (brighter_pixels >= (int) (((double) scan_size) * ratio));
        too_dark   = (darker_pixels   >= (int) (((double) scan_size) * ratio));

        LOG_DEBUG("Brighter = %0d, Darker = %0d (%s)\n",
                  brighter_pixels, darker_pixels,
                  (too_bright ? "Too Bright" :
                   too_dark   ? "Too Dark"   : "OK"));

        if (((cds_offset == cds_offset_upper) && (pga_gain == pga_gain_lower)) ||
            ((cds_offset == cds_offset_lower) && (pga_gain == pga_gain_upper))) {
            LOG_ERROR("No more settings!\n");
            break;
        }

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
f747b_detect_calibration(fps_handle_t *handle,
                         int          det_width,
                         int          det_height,
                         int          frms_to_susp)
{

    // Pre-defined parameters
    const int    frms_to_avg      = 4;
    const int    retry_limit      = 5;
    const int    pga_gain_init    = 0x02;
    const int    cds_offset_init  = FPS_MAX_CDS_OFFSET_1 / 2;
    const int    detect_th_upper  = FPS_MAX_DETECT_TH;
    const int    detect_th_lower  = FPS_MIN_DETECT_TH;
    const int    cds_offset_upper = FPS_MAX_CDS_OFFSET_1;
    const int    cds_offset_lower = FPS_MIN_CDS_OFFSET_1;
    const double sleep_mul        = 1.5;

    int     status = 0;
    uint8_t addr[3];
    uint8_t data[3];
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
    double  cds_offset_step_mv;
    int     cds_offset_tol;
    double  sleep_us;
    int     retry_cnt;

    // 1. Set suspend interval
    // ------------------------------------------------------------------------

    FPS_DISABLE_AND_CLEAR_INTERRUPT(handle, FPS_ALL_EVENTS);

    // Set suspend interval
    status = fps_set_suspend_frames(handle, frms_to_susp);
    if (status < 0) {
        return status;
    }

    // Calculate corresponding sleep time
    sleep_us  = fps_calculate_suspend_time((det_width * det_height), frms_to_susp);
    sleep_us *= sleep_mul;
    LOG_DEBUG("Sleep Time = %0.3f us\n", sleep_us);


    // 2. Set initial CDS and PGA settings
    // ------------------------------------------------------------------------

    addr[0] = FPS_REG_DET_CDS_CTL_0;
    addr[1] = FPS_REG_DET_CDS_CTL_1;
    addr[2] = FPS_REG_DET_PGA1_CTL;

    status = fps_multiple_read(handle, addr, data, 3);
    if (status < 0) {
        return status;
    }

    data[0] = ((uint8_t) ((cds_offset_init & 0x100) >> 1)) | (data[0] & 0x7F);
    data[1] = (uint8_t) ((cds_offset_init & 0x0FF) >> 0);
    data[2] = ((uint8_t) (pga_gain_init & 0x0F)) | (data[2] & 0xF0);

    status = fps_multiple_write(handle, addr, data, 3);
    if (status < 0) {
        return status;
    }

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
                                      frms_to_avg,
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

    // Check CDS current setting
    status = fps_single_read(handle, FPS_REG_ANA_I_SET_0, &data[0]);
    if (status < 0) {
        return status;
    }

    switch (data[0] & 0x03) {
        case 0x00 : cds_offset_step_mv = 1.5; break;
        case 0x01 : cds_offset_step_mv = 2.0; break;
        case 0x02 : cds_offset_step_mv = 2.0; break;
        default   : cds_offset_step_mv = 2.5; break;
    }

    // Calculate the CDS Offset tolerance
    cds_offset_tol = (int) ((sqrt(det_var)       *  // ADC code deviation
                             1                   /  // 1 sigma
                             256 * 3.3 * 1000    /  // ADC code -> mV
                             cds_offset_step_mv) +  // CDS offset per step
                            0.5);
    LOG_DEBUG("CDS Offset Tolerance = %0d\n", cds_offset_tol);

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
        LOG_ERROR("Retry time out! Exit...\n");
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
                                              1,
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
        LOG_ERROR("Retry time out! Exit...\n");
        return -1;
    }

    // 6. Final check
    // -------------------------------------------------------------------------

    status = fps_finetune_detect_cds_offset_upward(handle,
                                                   cds_offset_upper,
                                                   cds_offset_lower,
                                                   1,
                                                   sleep_us,
                                                   8,
                                                   &cds_offset);
    if (status < 0) {
        return status;
    }

    cds_offset++;
    if (cds_offset > FPS_MAX_CDS_OFFSET_1) {
        cds_offset = FPS_MAX_CDS_OFFSET_1;
    }

    data[0] = ((uint8_t) ((cds_offset & 0x100) >> 1)) | (data[0] & 0x7F);
    data[1] =  (uint8_t) ((cds_offset & 0x0FF) >> 0);

    status = fps_multiple_write(handle, addr, data, 2);

    return status;
}


