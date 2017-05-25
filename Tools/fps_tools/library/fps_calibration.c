#include <stdlib.h>
#include <math.h>
#include "common.h"
#include "debug.h"
#include "fps_register.h"
#include "fps_control.h"
#include "fps_calibration.h"


////////////////////////////////////////////////////////////////////////////////
//
// Image Calibration
//

int
fps_image_calibration(fps_handle_t *handle,
                      int          img_width,
                      int          img_height,
                      int          frms_to_avg)
{
    if (handle->image_calibration_method == NULL) {
        return -1;
    }

    return handle->image_calibration_method(handle,
                                            img_width,
                                            img_height,
                                            frms_to_avg);
}

int
fps_set_image_calibration_callback(fps_handle_t       *handle,
                                   fps_cal_callback_t callback)
{
    handle->image_calibration_callback = callback;
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
// Detect Calibration
//

int
fps_detect_calibration(fps_handle_t *handle,
                       int           det_width,
                       int           det_height,
                       int           frms_to_susp)
{
    if (handle->detect_calibration_method == NULL) {
        return -1;
    }

    return handle->detect_calibration_method(handle,
                                             det_width,
                                             det_height,
                                             frms_to_susp);
}

int
fps_set_detect_calibration_callback(fps_handle_t       *handle,
                                    fps_cal_callback_t callback)
{
    handle->detect_calibration_callback = callback;
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
// Search Detect Window
//

int
fps_search_detect_window(fps_handle_t *handle,
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
                         double       *det_noise)
{
    // Pre-defined parameters
    const double var_upper   = 5.0;
	const double noise_upper = 1.0;

    int            status = 0;
    size_t         img_size;
    size_t         det_size;
    int            ver_scan_cnt;
    int            hor_scan_cnt;
    int            total_scan_cnt;
    uint8_t        addr[6];
    uint8_t        data[6];
	int            mode_old;
    uint8_t        **img_array = NULL;
	uint8_t        *out_img    = NULL;
    double         *avg_img    = NULL;
    double         *noise_img  = NULL;
    double         *win_avg    = NULL;
    double         *win_var    = NULL;
    double         *win_noise  = NULL;
    int            col_start;
    int            row_start;
    double         pix_val;
    double         pix_sum;
    double         sqr_sum;
	double         noise_sum;
    double         avg;
    double         var;
	double         noise;
	fps_cal_info_t info;
    int            i;
    int            f;
    int            w;
    int            sc;
    int            sr;
    int            c;
    int            r;

    img_size = img_width * img_height;
    det_size = det_width * det_height;

    hor_scan_cnt   = scan_width  - det_width  + 1;
    ver_scan_cnt   = scan_height - det_height + 1;
    total_scan_cnt = hor_scan_cnt * ver_scan_cnt;

    addr[0] = FPS_REG_IMG_CDS_CTL_0;
    addr[1] = FPS_REG_IMG_CDS_CTL_1;
    addr[2] = FPS_REG_IMG_PGA1_CTL;

    addr[3] = FPS_REG_DET_CDS_CTL_0;
    addr[4] = FPS_REG_DET_CDS_CTL_1;
    addr[5] = FPS_REG_DET_PGA1_CTL;

    status = fps_multiple_read(handle, addr, data, 6);
    if (status < 0) {
        goto fps_search_detect_window_end;
    }

    status = fps_multiple_write(handle, addr, &data[3], 3);
    if (status < 0) {
        goto fps_search_detect_window_end;
    }

    status = fps_switch_sensor_mode(handle, FPS_IMAGE_MODE, &mode_old);
    if (status < 0) {
        goto fps_search_detect_window_end;
    }

    img_array = (uint8_t **) malloc(sizeof(uint8_t *) * frames);
    if (img_array == NULL) {
        status = -1;
        goto fps_search_detect_window_end;
    }

	for (f = 0; f < frames; f++) {
		img_array[f] = NULL;
        img_array[f] = (uint8_t *) malloc(sizeof(uint8_t) * img_size);
        if (img_array[f] == NULL) {
            status = -1;
            goto fps_search_detect_window_end;
        }
	}

    for (f = 0; f < frames; f++) {
        status = fps_get_raw_image(handle,
                                   img_width,
                                   img_height,
                                   img_array[f]);
        if (status < 0) {
            goto fps_search_detect_window_end;
        }
    }

    status = fps_switch_sensor_mode(handle, mode_old, NULL);
    if (status < 0) {
        goto fps_search_detect_window_end;
    }

    status = fps_multiple_write(handle, addr, data, 3);
    if (status < 0) {
        goto fps_search_detect_window_end;
    }

    avg_img = (double *) malloc(sizeof(double) * img_size);
    if (avg_img == NULL) {
        status = -1;
        goto fps_search_detect_window_end;
    }

    noise_img = (double *) malloc(sizeof(double) * img_size);
    if (noise_img == NULL) {
        status = -1;
        goto fps_search_detect_window_end;
    }

    for (i = 0; i < (int) img_size; i++) {
        pix_sum = 0.0;
        sqr_sum = 0.0;
        for (f = 0; f < frames; f++) {
            pix_val  = (double) img_array[f][i];
            pix_sum += pix_val;
            sqr_sum += SQUARE(pix_val);
        }

        avg_img[i]   = pix_sum / frames;
        noise_img[i] = sqrt((sqr_sum / frames) - SQUARE(avg_img[i]));
    }

	if (handle->detect_calibration_callback != NULL) {
		out_img = (uint8_t *) malloc(sizeof(uint8_t) * img_size);
		if (out_img == NULL) {
			status = -1;
			goto fps_search_detect_window_end;
		}

		for (i = 0; i < (int) img_size; i++) {
			out_img[i] = (uint8_t) avg_img[i]; 
		}

		info.img_buf      = out_img;
        info.cds_offset_1 = 0;
        info.pga_gain_1   = 0;
        info.detect_th    = 0;

		status = handle->detect_calibration_callback(handle, &info);
		if (status < 0) {
			goto fps_search_detect_window_end;
		}

		info.img_buf = NULL;
	}

    // Create to record all detect windows' average and variance 
    win_avg = (double *) malloc(sizeof(double) * total_scan_cnt);
    if (win_avg == NULL) {
        status = -1;
        goto fps_search_detect_window_end;
    }

    win_var = (double *) malloc(sizeof(double) * total_scan_cnt);
    if (win_var == NULL) {
        status = -1;
        goto fps_search_detect_window_end;
    }

    win_noise = (double *) malloc(sizeof(double) * total_scan_cnt);
    if (win_noise == NULL) {
        status = -1;
        goto fps_search_detect_window_end;
    }

    for (w = 0; w < total_scan_cnt; w++) {
        win_avg[w]   = 0.0;
        win_var[w]   = 0.0;
        win_noise[w] = 0.0;
    }

    col_start = (img_width  - scan_width ) / 2;
    row_start = (img_height - scan_height) / 2;

    // Walk through all candidate windows vertically
    for (sr = row_start; sr < (row_start + ver_scan_cnt); sr++) {
        // Walk through all candidate windows horizontally
        for (sc = col_start; sc < (col_start + hor_scan_cnt); sc++) {
    
            // Calculate average and variance of candidate window
            pix_sum   = 0.0;
            sqr_sum   = 0.0;
            noise_sum = 0.0;
            for (r = sr; r < (sr + det_height); r++) {
            for (c = sc; c < (sc + det_width ); c++) {
                pix_val    = avg_img[r * img_width + c];
                pix_sum   += pix_val;
                sqr_sum   += SQUARE(pix_val);
                noise_sum += noise_img[r * img_width + c];
            }}
    
            w = (sr - row_start) * hor_scan_cnt + (sc - col_start);

            win_avg[w]   = pix_sum / det_size;
            win_var[w]   = (sqr_sum / det_size) - SQUARE(win_avg[w]);
            win_noise[w] = noise_sum / det_size;
    
            LOG_DEBUG("Window (CB,RB):(CE,RE) = (%0d,%0d):(%0d,%0d) Avg. = %0.3f, Var. = %0.3f, Noise = %0.3f\n",
                      sc, sr, (sc + det_width - 1), (sr + det_height - 1), win_avg[w], win_var[w], win_noise[w]);
        }
    }

    // Search the window with maximum variance
    *det_col_begin = col_start;
    *det_col_end   = col_start + det_width  - 1;
    *det_row_begin = row_start;
    *det_row_end   = row_start + det_height - 1;

	avg   = win_avg[0];
    var   = win_var[0];
    noise = win_noise[0];

    // Walk through all candidate windows vertically
    for (sr = row_start; sr < (row_start + ver_scan_cnt); sr++) {
        // Walk through all candidate windows horizontally
        for (sc = col_start; sc < (col_start + hor_scan_cnt); sc++) {
       
            w = (sr - row_start) * hor_scan_cnt + (sc - col_start);
       
            // Find a window with minimum noise
            if (win_noise[w] < noise) {
                *det_col_begin = sc;
                *det_col_end   = sc + det_width  - 1;
                *det_row_begin = sr;
                *det_row_end   = sr + det_height - 1;
       
                avg   = win_avg[w];
                var   = win_var[w];
                noise = win_noise[w];
            }
        }
    }

    if ((var >= var_upper) || (noise >= noise_upper)) {
        LOG_DEBUG("Too bad to search a good detect window!\n");
        status = -1;
        goto fps_search_detect_window_end;
    }

    if (det_avg != NULL) {
        *det_avg = avg;
    }

    if (det_var != NULL) {
        *det_var = var;
    }

    if (det_noise != NULL) {
        *det_noise = noise;
    }

	LOG_DEBUG("\n");
	LOG_DEBUG("Result:\n");
    LOG_DEBUG("Window (CB,RB):(CE,RE) = (%0d,%0d):(%0d,%0d) Avg. = %0.3f, Var. = %0.3f, Noise = %0.3f\n",
              *det_col_begin, *det_row_begin, *det_col_end, *det_row_end, avg, var, noise);
	LOG_DEBUG("\n");

fps_search_detect_window_end:

    if (img_array != NULL) {
        for (f = 0; f < frames; f++) {
            if (img_array[f] != NULL) {
                free(img_array[f]);
            }
        }
        free(img_array);
    }

	if (out_img != NULL) {
		free(out_img);
	}

    if (avg_img != NULL) {
        free(avg_img);
    }

    if (noise_img != NULL) {
        free(noise_img);
    }

    if (win_avg != NULL) {
        free(win_avg);
    }

    if (win_var != NULL) {
        free(win_var);
    }

    if (win_noise != NULL) {
        free(win_noise);
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Search Detect Threshold
// -----------------------------------------------------------------------------
// NOTE:
//

int
fps_search_detect_threshold(fps_handle_t *handle,
                            int          search_mode,
                            int          upper_bound,
                            int          lower_bound,
                            double       sleep_us,
                            int          scan_limit,
                            int          *detect_th)
{
    int            status = 0;
    uint8_t        data;
    int            cds_offset;
    int            detect_th_upper;
    int            detect_th_middle;
    int            detect_th_lower;
    int            scan_cnt;
    fps_cal_info_t info;

    if (handle->detect_calibration_callback != NULL) {
        status = fps_get_sensor_parameter(handle,
                                          (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_1),
                                          &cds_offset);
        if (status < 0) {
            return status;
        }
    }

    status = fps_single_read(handle, FPS_REG_V_DET_SEL, &data);
    if (status < 0) {
        return status;
    }

    detect_th_upper  = upper_bound;
    detect_th_lower  = lower_bound;
    detect_th_middle = ((upper_bound + lower_bound) / 2) +
                       ((upper_bound + lower_bound) % 2);

    while ((detect_th_upper - detect_th_lower) > 1) {
        LOG_DEBUG("Detect Threshold range = 0x%02X : 0x%02X : 0x%02X\n",
                  detect_th_upper, detect_th_middle, detect_th_lower);

        data = ((uint8_t) ((detect_th_middle & 0x3F) >> 0)) | (data & 0xC0);
        status = fps_single_write(handle, FPS_REG_V_DET_SEL, data);
        if (status < 0) {
            return status;
        }

		(void) fps_scan_detect_event(handle, sleep_us);

        for (scan_cnt = 0; scan_cnt < scan_limit; scan_cnt++) {
            status = fps_scan_detect_event(handle, sleep_us);
            if (status < 0) {
                return status;
            }

            if (search_mode == FPS_SEARCH_ONE_TRIGGER) {
                if (status > 0) {
                    break;
                }
            } else {
                if (status == 0) {
                    break;
                }
            }
        }

        if (search_mode == FPS_SEARCH_ONE_TRIGGER) {
            if ((scan_cnt == scan_limit) && (status == 0)) {
                detect_th_upper = detect_th_middle;
            } else {
                detect_th_lower = detect_th_middle;
            }
        } else {
            if ((scan_cnt == scan_limit) && (status > 0)) {
                detect_th_lower = detect_th_middle;
            } else {
                detect_th_upper = detect_th_middle;
            }
        }
        
        if (handle->detect_calibration_callback != NULL) {
            info.detect_th    = detect_th_middle;
            info.cds_offset_1 = cds_offset;

            status = handle->detect_calibration_callback(handle, &info);
            if (status < 0) {
                return status;
            }
        }

        detect_th_middle = ((detect_th_upper + detect_th_lower) / 2) +
                           ((detect_th_upper + detect_th_lower) % 2);
    }

    *detect_th = detect_th_upper;

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Fine Tune Detect Threshold, Downward
// -----------------------------------------------------------------------------
// NOTE:
//

int
fps_finetune_detect_threshold_downward(fps_handle_t *handle,
                                       int          upper_bound,
                                       int          lower_bound,
									   int          check_range,
                                       double       sleep_us,
                                       int          scan_limit,
                                       int          *detect_th)
{
    int     status = 0;
    int     cds_offset;
    uint8_t data;
    int     score;
    int     scan_cnt;
    int     too_insensitive;

    if (handle->detect_calibration_callback != NULL) {
        status = fps_get_sensor_parameter(handle,
                                          (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_1),
                                          &cds_offset);
        if (status < 0) {
            return status;
        }
    }

    status = fps_single_read(handle, FPS_REG_V_DET_SEL, &data);
    if (status < 0) {
        return status;
    }

    for (score = scan_limit; score > 0; score--) {
        if ((*detect_th + check_range) > upper_bound) {
            data = ((uint8_t) ((upper_bound & 0x3F) >> 0)) | (data & 0xC0);
        } else {
            data = ((uint8_t) (((*detect_th + check_range) & 0x3F) >> 0)) | (data & 0xC0);
        }

        status = fps_single_write(handle, FPS_REG_V_DET_SEL, data);
        if (status < 0) {
            return status;
        }

		(void) fps_scan_detect_event(handle, sleep_us);

        for (scan_cnt = 0; scan_cnt < scan_limit; scan_cnt++) {
            status = fps_scan_detect_event(handle, sleep_us);
            if (status < 0) {
                return status;
            }

            if (status == 0) {
                if ((*detect_th) != lower_bound) {
                    LOG_DEBUG("Detect Threshold adjusted (--)!\n");
                    (*detect_th)--;
                } else {
                    LOG_ERROR("Minimum Detect Threshold reached!\n");
                    return -2;
                }
                break;
            }
        }

        if ((scan_cnt == scan_limit) && (status > 0)) {
            break;
        }
    }

    too_insensitive = (score < scan_limit);
    LOG_DEBUG("Score = %0d (%s)\n", score, ((too_insensitive == TRUE) ? "Insensitive" : "OK"));

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Fine Tune Detect Threshold, Upward
// -----------------------------------------------------------------------------
// NOTE:
//

int
fps_finetune_detect_threshold_upward(fps_handle_t *handle,
                                     int          upper_bound,
                                     int          lower_bound,
									 int          check_range,
                                     double       sleep_us,
                                     int          scan_limit,
                                     int          *detect_th)
{
    int     status = 0;
    int     cds_offset;
    uint8_t data;
    int     score;
    int     scan_cnt;
    int     too_sensitive;

    if (handle->detect_calibration_callback != NULL) {
        status = fps_get_sensor_parameter(handle,
                                          (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_1),
                                          &cds_offset);
        if (status < 0) {
            return status;
        }
    }

    status = fps_single_read(handle, FPS_REG_V_DET_SEL, &data);
    if (status < 0) {
        return status;
    }

    for (score = scan_limit; score > 0; score--) {
        if ((*detect_th - check_range) < lower_bound) {
            data = ((uint8_t) ((lower_bound & 0x3F) >> 0)) | (data & 0xC0);
        } else {
            data = ((uint8_t) (((*detect_th - check_range) & 0x3F) >> 0)) | (data & 0xC0);
        }

        status = fps_single_write(handle, FPS_REG_V_DET_SEL, data);
        if (status < 0) {
            return status;
        }

		(void) fps_scan_detect_event(handle, sleep_us);

        for (scan_cnt = 0; scan_cnt < scan_limit; scan_cnt++) {
            status = fps_scan_detect_event(handle, sleep_us);
            if (status < 0) {
                return status;
            }

            if (status > 0) {
                if ((*detect_th) != upper_bound) {
                    LOG_DEBUG("Detect Threshold adjusted (++)!\n");
                    (*detect_th)++;
                } else {
                    LOG_ERROR("Maximum Detect Threshold reached!\n");
                    return -2;
                }
                break;
            }
        }

        if ((scan_cnt == scan_limit) && (status == 0)) {
            break;
        }
    }

    too_sensitive = (score < scan_limit);
    LOG_DEBUG("Score = %0d (%s)\n", score, ((too_sensitive == TRUE) ? "Sensitive" : "OK"));

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Search Detect Mode CDS Offset
// -----------------------------------------------------------------------------
// NOTE:
//

int
fps_search_detect_cds_offset(fps_handle_t *handle,
                             int          search_mode,
                             int          upper_bound,
                             int          lower_bound,
                             double       sleep_us,
                             int          scan_limit,
                             int          *cds_offset)
{
    int            status = 0;
    uint8_t        addr[2];
    uint8_t        data[2];
    int            detect_th;
    int            cds_offset_upper;
    int            cds_offset_middle;
    int            cds_offset_lower;
    int            scan_cnt;
    fps_cal_info_t info;

    if (handle->detect_calibration_callback != NULL) {
        status = fps_get_sensor_parameter(handle,
                                          (FPS_DETECT_MODE | FPS_PARAM_DETECT_THRESHOLD),
                                          &detect_th);
        if (status < 0) {
            return status;
        }
    }

    addr[0] = FPS_REG_DET_CDS_CTL_0;
    addr[1] = FPS_REG_DET_CDS_CTL_1;
    
    status = fps_multiple_read(handle, addr, data, 2);
    if (status < 0) {
        return status;
    }

    cds_offset_upper  = upper_bound;
    cds_offset_lower  = lower_bound;
    cds_offset_middle = ((upper_bound + lower_bound) / 2) +
                        ((upper_bound + lower_bound) % 2);

    while ((cds_offset_upper - cds_offset_lower) > 1) {
        LOG_DEBUG("CDS Offset range = 0x%03X : 0x%03X : 0x%03X\n",
                  cds_offset_upper, cds_offset_middle, cds_offset_lower);

        data[0] = ((uint8_t) ((cds_offset_middle & 0x100) >> 1)) | (data[0] & 0x7F);
        data[1] = ((uint8_t) ((cds_offset_middle & 0x0FF) >> 0));

        status = fps_multiple_write(handle, addr, data, 2);
        if (status < 0) {
            return status;
        }

		(void) fps_scan_detect_event(handle, sleep_us);

        for (scan_cnt = 0; scan_cnt < scan_limit; scan_cnt++) {
            status = fps_scan_detect_event(handle, sleep_us);
            if (status < 0) {
                return status;
            }

            if (search_mode == FPS_SEARCH_ONE_TRIGGER) {
                if (status > 0) {
                    break;
                }
            } else {
                if (status == 0) {
                    break;
                }
            }
        }

        if (search_mode == FPS_SEARCH_ONE_TRIGGER) {
            if ((scan_cnt == scan_limit) && (status == 0)) {
                cds_offset_upper = cds_offset_middle;
            } else {
                cds_offset_lower = cds_offset_middle;
            }
        } else {
            if ((scan_cnt == scan_limit) && (status > 0)) {
                cds_offset_lower = cds_offset_middle;
            } else {
                cds_offset_upper = cds_offset_middle;
            }
        }
        
        if (handle->detect_calibration_callback != NULL) {
            info.detect_th    = detect_th;
            info.cds_offset_1 = cds_offset_middle;

            status = handle->detect_calibration_callback(handle, &info);
            if (status < 0) {
                return status;
            }
        }

        cds_offset_middle = ((cds_offset_upper + cds_offset_lower) / 2) +
                            ((cds_offset_upper + cds_offset_lower) % 2);
    }

    *cds_offset = cds_offset_upper;

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Fine Tune CDS Offset, Downward
// -----------------------------------------------------------------------------
// NOTE:
//

int
fps_finetune_detect_cds_offset_downward(fps_handle_t *handle,
                                        int          upper_bound,
                                        int          lower_bound,
									    int          check_range,
                                        double       sleep_us,
                                        int          scan_limit,
                                        int          *cds_offset)
{
    int     status = 0;
    int     detect_th;
    uint8_t addr[2];
    uint8_t data[2];
    int     score;
    int     scan_cnt;
    int     too_insensitive;

    if (handle->detect_calibration_callback != NULL) {
        status = fps_get_sensor_parameter(handle,
                                          (FPS_DETECT_MODE | FPS_PARAM_DETECT_THRESHOLD),
                                          &detect_th);
        if (status < 0) {
            return status;
        }
    }

    addr[0] = FPS_REG_DET_CDS_CTL_0;
    addr[1] = FPS_REG_DET_CDS_CTL_1;
    
    status = fps_multiple_read(handle, addr, data, 2);
    if (status < 0) {
        return status;
    }

    for (score = scan_limit; score > 0; score--) {
        if ((*cds_offset + check_range) > upper_bound) {
            data[0] = ((uint8_t) ((upper_bound & 0x100) >> 1)) | (data[0] & 0x7F);
            data[1] =  (uint8_t) ((upper_bound & 0x0FF) >> 0);
        } else {
            data[0] = ((uint8_t) (((*cds_offset + check_range) & 0x100) >> 1)) | (data[0] & 0x7F);
            data[1] =  (uint8_t) (((*cds_offset + check_range) & 0x0FF) >> 0);
        }

        status = fps_multiple_write(handle, addr, data, 2);
        if (status < 0) {
            return status;
        }

		(void) fps_scan_detect_event(handle, sleep_us);

        for (scan_cnt = 0; scan_cnt < scan_limit; scan_cnt++) {
            status = fps_scan_detect_event(handle, sleep_us);
            if (status < 0) {
                return status;
            }

            if (status == 0) {
                if ((*cds_offset) != lower_bound) {
                    LOG_DEBUG("CDS Offset adjusted (--)!\n");
                    (*cds_offset)--;
                } else {
                    LOG_ERROR("Minimum CDS Offset reached!\n");
                    return -2;
                }
                break;
            }
        }

        if ((scan_cnt == scan_limit) && (status > 0)) {
            break;
        }
    }

    too_insensitive = (score < scan_limit);
    LOG_DEBUG("Score = %0d (%s)\n", score, ((too_insensitive == TRUE) ? "Insensitive" : "OK"));

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Fine Tune CDS Offset, Upward
// -----------------------------------------------------------------------------
// NOTE:
//

int
fps_finetune_detect_cds_offset_upward(fps_handle_t *handle,
                                      int          upper_bound,
                                      int          lower_bound,
									  int          check_range,
                                      double       sleep_us,
                                      int          scan_limit,
                                      int          *cds_offset)
{
    int     status = 0;
    int     detect_th;
    uint8_t addr[2];
    uint8_t data[2];
    int     score;
    int     scan_cnt;
    int     too_sensitive;

    if (handle->detect_calibration_callback != NULL) {
        status = fps_get_sensor_parameter(handle,
                                          (FPS_DETECT_MODE | FPS_PARAM_DETECT_THRESHOLD),
                                          &detect_th);
        if (status < 0) {
            return status;
        }
    }

    addr[0] = FPS_REG_DET_CDS_CTL_0;
    addr[1] = FPS_REG_DET_CDS_CTL_1;
    
    status = fps_multiple_read(handle, addr, data, 2);
    if (status < 0) {
        return status;
    }

    for (score = scan_limit; score > 0; score--) {
        if ((*cds_offset - check_range) < lower_bound) {
            data[0] = ((uint8_t) ((lower_bound & 0x100) >> 1)) | (data[0] & 0x7F);
            data[1] =  (uint8_t) ((lower_bound & 0x0FF) >> 0);
        } else {
            data[0] = ((uint8_t) (((*cds_offset - check_range) & 0x100) >> 1)) | (data[0] & 0x7F);
            data[1] =  (uint8_t) (((*cds_offset - check_range) & 0x0FF) >> 0);
        }

        status = fps_multiple_write(handle, addr, data, 2);
        if (status < 0) {
            return status;
        }

		(void) fps_scan_detect_event(handle, sleep_us);

        for (scan_cnt = 0; scan_cnt < scan_limit; scan_cnt++) {
            status = fps_scan_detect_event(handle, sleep_us);
            if (status < 0) {
                return status;
            }

            if (status > 0) {
                if ((*cds_offset) != upper_bound) {
                    LOG_DEBUG("CDS Offset adjusted (++)!\n");
                    (*cds_offset)++;
                } else {
                    LOG_ERROR("Maximum CDS Offset reached!\n");
                    return -2;
                }
                break;
            }
        }

        if ((scan_cnt == scan_limit) && (status == 0)) {
            break;
        }
    }

    too_sensitive = (score < scan_limit);
    LOG_DEBUG("Score = %0d (%s)\n", score, ((too_sensitive == TRUE) ? "Sensitive" : "OK"));

    return status;
}


