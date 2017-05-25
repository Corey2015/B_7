#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "fps.h"
#include "fps_register.h"
#include "fps_control.h"
#include "cli.h"
#include "image.h"
#include "sleep.h"
#include "stopwatch.h"


extern fps_handle_t *device_handle;
extern char         image_folder[MAX_STRING_LENGTH];


////////////////////////////////////////////////////////////////////////////////
//
// Detect Calibration
//

static int
detect_calibration_callback(fps_handle_t   *handle,
							fps_cal_info_t *info)
{
    printf("     INFO: Detect Threshold = 0x%02X, CDS Offset 1 = 0x%03X\n",
           info->detect_th, info->cds_offset_1);

#if defined(__DEBUG__)
    {
        static int cnt = 0;
        char       file_name[MAX_STRING_LENGTH];
        if (info->img_buf != NULL) {
            sprintf(file_name, "%s/detect_calibration_progress_%0d.bmp", image_folder, cnt++);
            save_bmp(file_name, info->img_buf, FPS_SENSOR_SIZE);
        }
    }
#endif

    if (is_ctrl_c_hit() == TRUE) {
        return -1;
    } else {
        return 0;
    }
}

int
detect_calibration()
{
    const int DEFAULT_DETECT_WIDTH      = 8;
    const int DEFAULT_DETECT_HEIGHT     = 8;
    const int DEFAULT_FRAMES_TO_SUSPEND = 4;

    int         status = 0;
    char        cmd_line[MAX_STRING_LENGTH];
    char        cmd_key;
    char        *cmd_opt;
    int         sensor_width;
    int         sensor_height;
    int         det_width;
    int         det_height;
    size_t      det_size;
    int         frms_to_susp;
    double      sleep_us;
    int         detect_th;
    int         cds_offset;
    int         det_col_begin;
    int         det_col_end;
    int         det_row_begin;
    int         det_row_end;
    int         mode_old;
    stopwatch_t stopwatch;
    double      elapsed;

    sensor_width  = fps_get_sensor_width(device_handle);
    sensor_height = fps_get_sensor_height(device_handle);

    frms_to_susp = DEFAULT_FRAMES_TO_SUSPEND;

    det_width  = DEFAULT_DETECT_WIDTH;
    det_height = DEFAULT_DETECT_HEIGHT;
    det_size   = det_width * det_height;

    while (1) {

        clear_console();

        printf("\n");
        printf("====================\n");
        printf(" Detect Calibration \n");
        printf("====================\n");
        printf("\n");
        printf("    'd' <width> <height> - Specify the detect window dimension.          \n");
        printf("                             <width>  = The detect window width.         \n");
        printf("                             <height> = The detect window height.        \n");
        printf("                           (These numbers must be decimal.)              \n");
        printf("                                                                         \n");
        printf("    's' <frames>         - Specify the suspend interval in frames.       \n");
        printf("                             <frames> =  Suspend interval in frames.     \n");
        printf("                           (This number must be decimal.)                \n");
        printf("                                                                         \n");
        printf("    'g'                  - Get current detect calibration settings.      \n");
        printf("                                                                         \n");
        printf("    ENTER                - Do the detect calibration.                    \n");
        printf("                                                                         \n");
        printf("    'q'                  - Back to main menu.                            \n");
        printf("\n");
        printf("Please enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("mdsgq\n", cmd_key) == NULL) {
            printf("    ERROR: Invalid command!\n");
            sleep_ms(1000);
            continue;
        }

        if (cmd_key == 'q') {
            break;
        }

        printf("\n");
        printf("Result:\n");
        printf("\n");

        if (cmd_key == 'd') {
            cmd_opt    = strtok(cmd_line, " ");
            cmd_opt    = strtok(NULL, " ");
            det_width  = strtol(cmd_opt, NULL, 10);
            cmd_opt    = strtok(NULL, " ");
            det_height = strtol(cmd_opt, NULL, 10);

            det_size = det_width * det_height;

            if ((det_width < 1) || (det_width > sensor_width)) {
                printf("    ERROR: Detect window width must be >= 1 and <= %0d!\n",
                       sensor_width);

                det_width = DEFAULT_DETECT_WIDTH;

                sleep_ms(1000);
                continue;
            }

            if ((det_height < 1) || (det_height > sensor_height)) {
                printf("    ERROR: Detect window height must be >= 1 and <= %0d!\n",
                       sensor_height);

                det_height = DEFAULT_DETECT_HEIGHT;

                sleep_ms(1000);
                continue;
            }

            goto detect_calibration_show_options;
        }

        if (cmd_key == 's') {
            cmd_opt      = strtok(cmd_line, " ");
            cmd_opt      = strtok(NULL, " ");
            frms_to_susp = strtol(cmd_opt, NULL, 10);

            if (frms_to_susp < 1) {
                printf("    ERROR: Suspend interval must be >= 1!\n");

                frms_to_susp = DEFAULT_FRAMES_TO_SUSPEND;

                sleep_ms(1000);
                continue;
            }

            goto detect_calibration_show_options;
        }

        if (cmd_key == 'g') {

detect_calibration_show_options :

            sleep_us = fps_calculate_suspend_time(det_size, frms_to_susp);

            printf("    Detect Window Size = %0d (%0dx%0d)\n", det_size, det_width, det_height);
            printf("    Suspend Interval   = %0d frames (%0.3f us)\n", frms_to_susp, sleep_us);
        }

        if (cmd_key == '\n') {

            printf("    Calibrating...\n");

            start_ctrl_c_monitor();

            status = fps_set_detect_calibration_callback(device_handle,
                                                         detect_calibration_callback);
            if (status < 0) {
                return status;
            }

            stopwatch_start(&stopwatch);

            status = fps_switch_sensor_mode(device_handle, FPS_DETECT_MODE, &mode_old);
            if (status < 0) {
                return status;
            }

            status = fps_detect_calibration(device_handle,
                                            det_width,
                                            det_height,
                                            frms_to_susp);

            stopwatch_stop(&stopwatch);
            elapsed = stopwatch_get_elapsed(&stopwatch);

            stop_ctrl_c_monitor();

            if (status < 0) {
                printf("    Failed!\n");
            } else {
                // Get final detect window
                (void) fps_get_sensing_area(device_handle, FPS_DETECT_MODE,
                                            &det_col_begin, &det_col_end,
                                            &det_row_begin, &det_row_end);

                // Get final Detect Threshold
                (void) fps_get_sensor_parameter(device_handle,
                                                FPS_PARAM_DETECT_THRESHOLD,
                                                &detect_th);

                // Get final CDS Offset
                (void) fps_get_sensor_parameter(device_handle,
                                                (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_1),
                                                &cds_offset);

                printf("    Successful!\n");
                printf("\n");
                printf("    Window (CB,RB):(CE,RE) = (%0d,%0d):(%0d,%0d)\n",
                       det_col_begin, det_row_begin, det_col_end, det_row_end);
                printf("    Detect Threshold       = 0x%02X\n", detect_th);
                printf("    CDS Offset 1           = 0x%03X\n", cds_offset);
                printf("\n");
                printf("    Time Elapsed           = %0.3f ms\n", elapsed);
            }

            status = fps_switch_sensor_mode(device_handle, mode_old, NULL);
            if (status < 0) {
                return status;
            }
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Detect Mode Test
//

int
detect_mode_test()
{
    const int DEFAULT_FRAMES_TO_SUSPEND = 4;
    const int DEFAULT_TEST_INTERVAL     = 500;

    int    status = 0;
    char   cmd_line[MAX_STRING_LENGTH];
    char   cmd_key;
    char   *cmd_opt;
    char   last_cmd_line[MAX_STRING_LENGTH];
    int    col_begin;
    int    col_end;
    int    row_begin;
    int    row_end;
    int    det_width;
    int    det_height;
    size_t det_size;
    int    frms_to_susp;
    double sleep_us;
    char   adj_type[MAX_STRING_LENGTH];
    int    adj_step;
    int    cds_offset_0;
    int    cds_offset_1;
    int    pga_gain_0;
    int    pga_gain_1;
    int    detect_th;
    int    mode_old;
    int    test_interval;
    int    detect_cnt;

    frms_to_susp  = DEFAULT_FRAMES_TO_SUSPEND;
    test_interval = DEFAULT_TEST_INTERVAL;

    // Get detect scan window first
    status = fps_get_sensing_area(device_handle, FPS_DETECT_MODE,
                                  &col_begin, &col_end,
                                  &row_begin, &row_end);
    if (status < 0) {
        return status;
    }

    det_width  = col_end - col_begin + 1;
    det_height = row_end - row_begin + 1;
    det_size   = det_width * det_height;

    // Get CDS offset, PGA gain and Detect threshold
    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_0),
                                      &cds_offset_0);
    if (status < 0) {
        return status;
    }

    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_1),
                                      &cds_offset_1);
    if (status < 0) {
        return status;
    }

    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_DETECT_MODE | FPS_PARAM_PGA_GAIN_0),
                                      &pga_gain_0);
    if (status < 0) {
        return status;
    }

    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_DETECT_MODE | FPS_PARAM_PGA_GAIN_1),
                                      &pga_gain_1);
    if (status < 0) {
        return status;
    }

    status = fps_get_sensor_parameter(device_handle,
                                      FPS_PARAM_DETECT_THRESHOLD,
                                      &detect_th);
    if (status < 0) {
        return status;
    }

    while (1) {

        clear_console();

        printf("\n");
        printf("==================\n");
        printf(" Detect Mode Test \n");
        printf("==================\n");
        printf("\n");
        printf("    's' <frames>       - Specify the suspend interval in frames.         \n");
        printf("                           <frames> = Suspend interval in frames.        \n");
        printf("                         (This number must be decimal.)                  \n");
        printf("                                                                         \n");
        printf("    'v' <msec>         - Interval between successive two tests.          \n");
        printf("                           <msec> = Interval in msec.                    \n");
        printf("                         (This number must be decimal.)                  \n");
        printf("                                                                         \n");
        printf("    '+' <type> <steps> - Increment current CDS/PGA/DetectTh settings.    \n");
        printf("    '-' <type> <steps> - Decrement current CDS/PGA/DetectTh settings.    \n");
        printf("                           <type> = CDS, PGA or DetectTh types.          \n");
        printf("                             'c0': CDS 0                                 \n");
        printf("                             'c1': CDS 1                                 \n");
        printf("                             'p0': PGA 0                                 \n");
        printf("                             'p1': PGA 1                                 \n");
        printf("                             'dt': Detect Threshold                      \n");
        printf("                           <steps> = Increment/Decrement steps.          \n");
        printf("                         (This number must be decimal.)                  \n");
        printf("                                                                         \n");
        printf("    'r'                - Repeat the last '+' or '-' command again.       \n");
        printf("                                                                         \n");
        printf("    'g'                - Get current Detect Mode test settings.          \n");
        printf("                                                                         \n");
        printf("    ENTER              - Do Detect Mode test based on current settings.  \n");
        printf("                                                                         \n");
        printf("    CTRL+C             - Exit the test loop.                             \n");
        printf("                                                                         \n");
        printf("    'q'                - Back to main menu.                              \n");
        printf("\n");
        printf("Pleae enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("smv+-rgq\n", cmd_key) == NULL) {
            printf("    ERROR: Invalid command!\n");
            sleep_ms(1000);
            continue;
        }

        if (cmd_key == 'q') {
            break;
        }

        printf("\n");
        printf("Result:\n");
        printf("\n");

detect_mode_test_adjust_again :

        // Set Suspend Interval
        // ---------------------------------------------------------------------

        if (cmd_key == 's') {
            cmd_opt      = strtok(cmd_line, " ");
            cmd_opt      = strtok(NULL, " ");
            frms_to_susp = strtol(cmd_opt, NULL, 10);

            if (frms_to_susp < 1) {
                printf("    ERROR: Suspend interval must be >= 1!\n");

                frms_to_susp = DEFAULT_FRAMES_TO_SUSPEND;

                sleep_ms(1000);
                continue;
            }

            goto detect_mode_test_show_options;
        }

        // Set Sleep Time Between Two Tests
        // ---------------------------------------------------------------------

        if (cmd_key == 'v') {
            cmd_opt       = strtok(cmd_line, " ");
            cmd_opt       = strtok(NULL, " ");
            test_interval = strtol(cmd_opt, NULL, 10);

            if (test_interval < 0) {
                printf("    ERROR: Test Interval must be >= 0!\n");

                test_interval = DEFAULT_TEST_INTERVAL;

                sleep_ms(1000);
                continue;
            }

            goto detect_mode_test_show_options;
        }

        // Adjust CDS, PGA, or Detect Threshold
        // ---------------------------------------------------------------------

        if ((cmd_key == '+') || (cmd_key == '-')) {

            // Backup the last command line for repeat commands
            strncpy(last_cmd_line, cmd_line, sizeof(cmd_line));

            cmd_opt = strtok(cmd_line, " ");
            cmd_opt = strtok(NULL, " ");

            memset(adj_type, 0x00, sizeof(adj_type));
            strncpy(adj_type, cmd_opt, 2);

            cmd_opt  = strtok(NULL, " ");
            adj_step = strtol(cmd_opt, NULL, 10);

            // CDS Offset 0
            if (strcmp(adj_type, "c0") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_0),
                                                  &cds_offset_0);
                if (status < 0) {
                    return status;
                }

                switch (cmd_key) {
                    case '+' : cds_offset_0 += adj_step; break;
                    case '-' : cds_offset_0 -= adj_step; break;
                    default  : break;
                }

                if ((cds_offset_0 < FPS_MIN_CDS_OFFSET_0) ||
                    (cds_offset_0 > FPS_MAX_CDS_OFFSET_0)) {
                    printf("    WARNING: Out of adjustment range!\n");
                    break;
                }

                status = fps_set_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_0),
                                                  cds_offset_0);
                if (status < 0) {
                    return status;
                }
            }

            // CDS Offset 1
            if (strcmp(adj_type, "c1") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_1),
                                                  &cds_offset_1);
                if (status < 0) {
                    return status;
                }

                switch (cmd_key) {
                    case '+' : cds_offset_1 += adj_step; break;
                    case '-' : cds_offset_1 -= adj_step; break;
                    default  : break;
                }

                if ((cds_offset_1 < FPS_MIN_CDS_OFFSET_1) ||
                    (cds_offset_1 > FPS_MAX_CDS_OFFSET_1)) {
                    printf("    WARNING: Out of adjustment range!\n");
                    break;
                }

                status = fps_set_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_CDS_OFFSET_1),
                                                  cds_offset_1);
                if (status < 0) {
                    return status;
                }
            }

            // PGA Gain 0
            if (strcmp(adj_type, "p0") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_PGA_GAIN_0),
                                                  &pga_gain_0);
                if (status < 0) {
                    return status;
                }

                switch (cmd_key) {
                    case '+' : pga_gain_0 += adj_step; break;
                    case '-' : pga_gain_0 -= adj_step; break;
                    default  : break;
                }

                if ((pga_gain_0 < FPS_MIN_PGA_GAIN_0) ||
                    (pga_gain_0 > FPS_MAX_PGA_GAIN_0)) {
                    printf("    WARNING: Out of adjustment range!\n");
                    break;
                }

                status = fps_set_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_PGA_GAIN_0),
                                                  pga_gain_0);
                if (status < 0) {
                    return status;
                }
            }

            // PGA Gain 1
            if (strcmp(adj_type, "p1") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_PGA_GAIN_1),
                                                  &pga_gain_1);
                if (status < 0) {
                    return status;
                }

                switch (cmd_key) {
                    case '+' : pga_gain_1 += adj_step; break;
                    case '-' : pga_gain_1 -= adj_step; break;
                    default  : break;
                }

                if ((pga_gain_1 < FPS_MIN_PGA_GAIN_1) ||
                    (pga_gain_1 > FPS_MAX_PGA_GAIN_1)) {
                    printf("    WARNING: Out of adjustment range!\n");
                    break;
                }

                status = fps_set_sensor_parameter(device_handle,
                                                  (FPS_DETECT_MODE | FPS_PARAM_PGA_GAIN_1),
                                                  pga_gain_1);
                if (status < 0) {
                    return status;
                }
            }

            // Detect Threshold
            if (strcmp(adj_type, "dt") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  FPS_PARAM_DETECT_THRESHOLD,
                                                  &detect_th);
                if (status < 0) {
                    return status;
                }

                switch (cmd_key) {
                    case '+' : detect_th += adj_step; break;
                    case '-' : detect_th -= adj_step; break;
                    default  : break;
                }

                if ((detect_th < FPS_MIN_DETECT_TH) ||
                    (detect_th > FPS_MAX_DETECT_TH)) {
                    printf("    WARNING: Out of adjustment range!\n");
                    break;
                }

                status = fps_set_sensor_parameter(device_handle,
                                                  FPS_PARAM_DETECT_THRESHOLD,
                                                  detect_th);
                if (status < 0) {
                    return status;
                }
            }

            goto detect_mode_test_show_options;
        }

        if (cmd_key == 'r') {
            strncpy(cmd_line, last_cmd_line, sizeof(cmd_line));
            cmd_key = cmd_line[0];
            goto detect_mode_test_adjust_again;
        }

        // Get current settings
        // ---------------------------------------------------------------------

        if (cmd_key == 'g') {

detect_mode_test_show_options :

            sleep_us = fps_calculate_suspend_time(det_size, frms_to_susp);

            printf("    Row Begin            = %0d\n", row_begin);
            printf("    Row End              = %0d\n", row_end);
            printf("    Column Begin         = %0d\n", col_begin);
            printf("    COlumn End           = %0d\n", col_end);
            printf("    Detect Window Size   = %0d (%0dx%0d)\n", det_size, det_width, det_height);
            printf("\n");
            printf("    Suspend Interval     = %0d frames (%0.3f us)\n", frms_to_susp, sleep_us);
            printf("    Test Interval        = %0d msec.\n", test_interval);
            printf("\n");
            printf("    CDS Offset 0         = 0x%02X\n", cds_offset_0);
            printf("    CDS Offset 1         = 0x%03X\n", cds_offset_1);
            printf("    PGA Gain 0           = 0x%02X\n", pga_gain_0);
            printf("    PGA Gain 1           = 0x%02X\n", pga_gain_1);
            printf("    Detect Threshold     = 0x%02X\n", detect_th);
        }

        // Enter Detect Mode
        // ---------------------------------------------------------------------

        if (cmd_key == '\n') {
            printf("    Testing Detect Interrupt...\n");

            status = fps_set_suspend_frames(device_handle, frms_to_susp);
            if (status < 0) {
                return status;
            }

            sleep_us = fps_calculate_suspend_time(det_size, frms_to_susp);

            detect_cnt = 0;

            start_ctrl_c_monitor();

            status = fps_switch_sensor_mode(device_handle, FPS_DETECT_MODE, &mode_old);
            if (status < 0) {
                return status;
            }

            while (is_ctrl_c_hit() == FALSE) {
                status = fps_scan_detect_event(device_handle, sleep_us);
                if (status < 0) {
                    return status;
                }

                // Finger-on detected
                if (status > 0) {
                    time_t     raw_time;
                    struct tm *time_info;
                    char       time_str[MAX_STRING_LENGTH];

                    time(&raw_time);
                    time_info = localtime(&raw_time);
                    strftime(time_str, sizeof(time_str), "%F %T", time_info);

                    printf("    INFO: Finger-on event is detected! Count = %0d (%s)\n",
                           detect_cnt++, time_str);
                }

                // If no finger-on detected...
            }

            stop_ctrl_c_monitor();

            printf("    Done!\n");
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();
    }

    return status;
}
