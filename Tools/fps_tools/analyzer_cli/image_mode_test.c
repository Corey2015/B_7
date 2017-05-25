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
// Image Calibration
//

static int
image_calibration_callback(fps_handle_t   *handle,
						   fps_cal_info_t *info)
{
    printf("     INFO: CDS Offset 1 = 0x%03X, PGA Gain 1 = 0x%02X\n",
           info->cds_offset_1, info->pga_gain_1);

#if defined(__DEBUG__)
    {
        static int cnt = 0;
        char       file_name[MAX_STRING_LENGTH];
        if (info->img_buf != NULL) {
            sprintf(file_name, "%s/image_calibration_progress_%0d.bmp", image_folder, cnt++);
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
image_calibration()
{
    const int DEFAULT_FRAMES_TO_AVERAGE = 1;

    int         status = 0;
    char        cmd_line[MAX_STRING_LENGTH];
    char        cmd_key;
    char        *cmd_opt;
    int         sensor_width;
    int         sensor_height;
    int         img_width;
    int         img_height;
    int         frms_to_avg;
    int         mode_old;
    int         cds_offset;
    int         pga_gain;
    stopwatch_t stopwatch;
    double      elapsed;

    sensor_width  = fps_get_sensor_width(device_handle);
    sensor_height = fps_get_sensor_height(device_handle);

    img_width   = sensor_width;
    img_height  = sensor_height;
    frms_to_avg = DEFAULT_FRAMES_TO_AVERAGE;

    while (1) {

        clear_console();

        printf("\n");
        printf("===================\n");
        printf(" Image Calibration \n");
        printf("===================\n");
        printf("\n");
        printf("    'd' <width> <height> - Specify the image dimension.                  \n");
        printf("                             <width>  = The image width.                 \n");
        printf("                             <height> = The image height.                \n");
        printf("                           (These numbers must be decimal.)              \n");
        printf("                                                                         \n");
        printf("    'a' <frames>         - Specify how many frames to average an image.  \n");
        printf("                             <frames> = Number of frames to average.     \n");
        printf("                           (This number must be decimal.)                \n");
        printf("                                                                         \n");
        printf("    'g'                  - Get current image calibration settings.       \n");
        printf("                                                                         \n");
        printf("    ENTER                - Do the image calibration.                     \n");
        printf("                                                                         \n");
        printf("    'q'                  - Back to main menu.                            \n");
        printf("\n");
        printf("Please enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("mdagq\n", cmd_key) == NULL) {
            printf("    ERROR: Invalid command!\n");
            sleep_ms(1000);
            continue;
        }

        if (cmd_key == 'q') {
            break;
        }

        if (cmd_key == 'd') {
            cmd_opt    = strtok(cmd_line, " ");
            cmd_opt    = strtok(NULL, " ");
            img_width  = strtol(cmd_opt, NULL, 10);
            cmd_opt    = strtok(NULL, " ");
            img_height = strtol(cmd_opt, NULL, 10);

            if ((img_width < 1) || (img_width > sensor_width)) {
                printf("    ERROR: Image width must be >= 1 and <= %0d!\n",
                       sensor_width);

                img_width = sensor_width;

                sleep_ms(1000);
                continue;
            }

            if ((img_height < 1) || (img_height > sensor_height)) {
                printf("    ERROR: Image height must be >= 1 and <= %0d!\n",
                       sensor_height);

                img_height = sensor_height;

                sleep_ms(1000);
                continue;
            }

            goto image_calibration_show_options;
        }

        if (cmd_key == 'a') {
            cmd_opt     = strtok(cmd_line, " ");
            cmd_opt     = strtok(NULL, " ");
            frms_to_avg = strtol(cmd_opt, NULL, 10);

            if (frms_to_avg < 1) {
                printf("    ERROR: Frames to average must be >= 1!\n");

                frms_to_avg = DEFAULT_FRAMES_TO_AVERAGE;

                sleep_ms(1000);
                continue;
            }

            goto image_calibration_show_options;
        }

        if (cmd_key == 'g') {

image_calibration_show_options :

            printf("\n");
            printf("Result:\n");
            printf("\n");

            printf("    Image Dimension             = %0dx%0d\n", img_width, img_height);
            printf("    Number of Frames to Average = %0d\n",     frms_to_avg);
        }

        if (cmd_key == '\n') {

            printf("    Calibrating...\n");

            start_ctrl_c_monitor();

            status = fps_set_image_calibration_callback(device_handle,
                                                        image_calibration_callback);
            if (status < 0) {
                return status;
            }

            stopwatch_start(&stopwatch);

            status = fps_switch_sensor_mode(device_handle, FPS_IMAGE_MODE, &mode_old);
            if (status < 0) {
                return status;
            }

            status = fps_image_calibration(device_handle,
                                           img_width,
                                           img_height,
                                           frms_to_avg);

            stopwatch_stop(&stopwatch);
            elapsed = stopwatch_get_elapsed(&stopwatch);

            stop_ctrl_c_monitor();

            if (status < 0) {
                printf("    Failed!\n");
            } else {
                // Get final CDS offset
                (void) fps_get_sensor_parameter(device_handle,
                                                (FPS_IMAGE_MODE | FPS_PARAM_CDS_OFFSET_1),
                                                &cds_offset);

                // Get final PGA gain
                (void) fps_get_sensor_parameter(device_handle,
                                                (FPS_IMAGE_MODE | FPS_PARAM_PGA_GAIN_1),
                                                &pga_gain);

                printf("    Successful!\n");
                printf("\n");
                printf("    CDS Offset 1 = 0x%03X  \n", cds_offset);
                printf("    PGA Gain 1   = 0x%02X  \n", pga_gain);
                printf("\n");
                printf("    Time Elapsed = %0.3f ms\n", elapsed);
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
// Image Mode Test
//

int
image_mode_test()
{
    const int DEFAULT_FRAMES_TO_AVERAGE   = 1;
    const int DEFAULT_NUMBER_OF_IMAGES    = 100;
    const int DEFAULT_ACQUIRED_IMAGE_TYPE = 1;    // Finger-only image

    int         status = 0;
    char        cmd_line[MAX_STRING_LENGTH];
    char        cmd_key;
    char        *cmd_opt;
    char        last_cmd_line[MAX_STRING_LENGTH];
    int         col_begin;
    int         col_end;
    int         row_begin;
    int         row_end;
    int         img_width;
    int         img_height;
    int         img_size;
    int         frms_to_avg;
    int         num_of_imgs;
    int         img_type;
    char        adj_type[MAX_STRING_LENGTH];
    int         adj_step;
    int         cds_offset_0;
    int         cds_offset_1;
    int         pga_gain_0;
    int         pga_gain_1;
    int         mode_old;
    char        file_name[MAX_FNAME_LENGTH];
    FILE        *fptr;
    uint8_t     addr[5];
    uint8_t     data[5];
    uint8_t     *bkgnd_img;
    double      bkgnd_avg;
    uint8_t     *frgnd_img;
    double      frgnd_avg;
    uint8_t     *finger_img;
    double      finger_dr;
    stopwatch_t stopwatch;
    double      elapsed;
    int         n;

    frms_to_avg = DEFAULT_FRAMES_TO_AVERAGE;
    num_of_imgs = DEFAULT_NUMBER_OF_IMAGES;
    img_type    = DEFAULT_ACQUIRED_IMAGE_TYPE;

    // Get image window first
    status = fps_get_sensing_area(device_handle, FPS_IMAGE_MODE,
                                  &col_begin, &col_end,
                                  &row_begin, &row_end);
    if (status < 0) {
        return status;
    }

    img_width  = col_end - col_begin + 1;
    img_height = row_end - row_begin + 1;
    img_size   = img_width * img_height;

    // Get CDS offset and PGA gain
    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_IMAGE_MODE | FPS_PARAM_CDS_OFFSET_0),
                                      &cds_offset_0);
    if (status < 0) {
        return status;
    }

    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_IMAGE_MODE | FPS_PARAM_CDS_OFFSET_1),
                                      &cds_offset_1);
    if (status < 0) {
        return status;
    }

    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_IMAGE_MODE | FPS_PARAM_PGA_GAIN_0),
                                      &pga_gain_0);
    if (status < 0) {
        return status;
    }

    status = fps_get_sensor_parameter(device_handle,
                                      (FPS_IMAGE_MODE | FPS_PARAM_PGA_GAIN_1),
                                      &pga_gain_1);
    if (status < 0) {
        return status;
    }

    bkgnd_img  = NULL;
    frgnd_img  = NULL;
    finger_img = NULL;

    while (1) {

        clear_console();

        printf("\n");
        printf("=================\n");
        printf(" Image Mode Test \n");
        printf("=================\n");
        printf("\n");
        printf("    'a' <frames>       - Specify how many frames to average.             \n");
        printf("                           <frames> = Number of frames to average.       \n");
        printf("                         (This number must be decimal.)                  \n");
        printf("                                                                         \n");
        printf("    'n' <images>       - Specify how many images to acquire in this test.\n");
        printf("                           <images> = Number of image to acquire .       \n");
        printf("                         (This number must be decimal.)                  \n");
        printf("                                                                         \n");
        printf("    't' <type>         - Specify the interested image type.              \n");
        printf("                           <type> = Image type identifiers.              \n");
        printf("                             0: Foreground raw images.                   \n");
        printf("                             1: Finger-only images.                      \n");
        printf("                             2: Enhanced images.                         \n");
        printf("                                                                         \n");
        printf("    '+' <type> <steps> - Increment current CDS/PGA settings.             \n");
        printf("    '-' <type> <steps> - Decrement current CDS/PGA settings.             \n");
        printf("                           <type> = CDS or PGA types.                    \n");
        printf("                             'c0': CDS 0                                 \n");
        printf("                             'c1': CDS 1                                 \n");
        printf("                             'p0': PGA 0                                 \n");
        printf("                             'p1': PGA 1                                 \n");
        printf("                           <steps> = Increment/Decrement steps.          \n");
        printf("                         (This number must be decimal.)                  \n");
        printf("                                                                         \n");
        printf("    'r'                - Repeat the last '+' or '-' command again.       \n");
        printf("                                                                         \n");
        printf("    'g'                - Get current getting image settings.             \n");
        printf("                                                                         \n");
        printf("    'b'                - Get an image and save it as a background.       \n");
        printf("                                                                         \n");
        printf("    ENTER              - Acquire images based on current settings.       \n");
        printf("                                                                         \n");
        printf("    'q'                - Back to main menu.                              \n");
        printf("\n");
        printf("Pleae enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("ant+-rgbq\n", cmd_key) == NULL) {
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

image_mode_test_adjust_again :

        // Set How Many Frames to Average
        // ---------------------------------------------------------------------

        if (cmd_key == 'a') {
            cmd_opt     = strtok(cmd_line, " ");
            cmd_opt     = strtok(NULL, " ");
            frms_to_avg = strtol(cmd_opt, NULL, 10);

            if (frms_to_avg < 1) {
                printf("    ERROR: Frames to average must be >= 1!\n");

                frms_to_avg = DEFAULT_FRAMES_TO_AVERAGE;

                sleep_ms(1000);
                continue;
            }

            goto image_mode_test_show_options;
        }

        // Set How Many Images to Acquire
        // ---------------------------------------------------------------------

        if (cmd_key == 'n') {
            cmd_opt     = strtok(cmd_line, " ");
            cmd_opt     = strtok(NULL, " ");
            num_of_imgs = strtol(cmd_opt, NULL, 10);

            if (num_of_imgs < 1) {
                printf("    ERROR: Number of images must be >= 1!\n");

                num_of_imgs = DEFAULT_NUMBER_OF_IMAGES;

                sleep_ms(1000);
                continue;
            }

            goto image_mode_test_show_options;
        }

        // Set Accquired Image Type
        // ---------------------------------------------------------------------

        if (cmd_key == 't') {
            cmd_opt  = strtok(cmd_line, " ");
            cmd_opt  = strtok(NULL, " ");
            img_type = strtol(cmd_opt, NULL, 10);

            if (img_type < 0) {
                printf("    ERROR: Image type identifier must be >= 0!\n");

                img_type = DEFAULT_ACQUIRED_IMAGE_TYPE;

                sleep_ms(1000);
                continue;
            }

            goto image_mode_test_show_options;
        }

        // Adjust CDS or PGA
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
                                                  (FPS_IMAGE_MODE | FPS_PARAM_CDS_OFFSET_0),
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
                                                  (FPS_IMAGE_MODE | FPS_PARAM_CDS_OFFSET_0),
                                                  cds_offset_0);
                if (status < 0) {
                    return status;
                }
            }

            // CDS Offset 1
            if (strcmp(adj_type, "c1") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  (FPS_IMAGE_MODE | FPS_PARAM_CDS_OFFSET_1),
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
                                                  (FPS_IMAGE_MODE | FPS_PARAM_CDS_OFFSET_1),
                                                  cds_offset_1);
                if (status < 0) {
                    return status;
                }
            }

            // PGA Gain 0
            if (strcmp(adj_type, "p0") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  (FPS_IMAGE_MODE | FPS_PARAM_PGA_GAIN_0),
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
                                                  (FPS_IMAGE_MODE | FPS_PARAM_PGA_GAIN_0),
                                                  pga_gain_0);
                if (status < 0) {
                    return status;
                }
            }

            // PGA Gain 1
            if (strcmp(adj_type, "p1") == 0) {
                status = fps_get_sensor_parameter(device_handle,
                                                  (FPS_IMAGE_MODE | FPS_PARAM_PGA_GAIN_1),
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
                                                  (FPS_IMAGE_MODE | FPS_PARAM_PGA_GAIN_1),
                                                  pga_gain_1);
                if (status < 0) {
                    return status;
                }
            }

            goto image_mode_test_show_options;
        }

        if (cmd_key == 'r') {
            strncpy(cmd_line, last_cmd_line, sizeof(cmd_line));
            cmd_key = cmd_line[0];
            goto image_mode_test_adjust_again;
        }

        // Get current settings
        // ---------------------------------------------------------------------

        if (cmd_key == 'g') {

image_mode_test_show_options :

            printf("    Row Begin    = %0d\n", row_begin);
            printf("    Row End      = %0d\n", row_end);
            printf("    Column Begin = %0d\n", col_begin);
            printf("    COlumn End   = %0d\n", col_end);
            printf("    Image Size   = %0d (%0dx%0d)\n", img_size, img_width, img_height);
            printf("\n");
            printf("    Number of Frames to Average = %0d frames\n", frms_to_avg);
            printf("    Number of Images to Acquire = %0d images\n", num_of_imgs);
            printf("    Acquired Image Type         = %0d\n", img_type);
            printf("\n");
            printf("    CDS Offset 0 = 0x%02X\n", cds_offset_0);
            printf("    CDS Offset 1 = 0x%03X\n", cds_offset_1);
            printf("    PGA Gain 0   = 0x%02X\n", pga_gain_0);
            printf("    PGA Gain 1   = 0x%02X\n", pga_gain_1);
        }

        // Get Background
        // ---------------------------------------------------------------------

        if (cmd_key == 'b') {

            printf("    Getting Background Image...\n");

            bkgnd_img = (uint8_t *) malloc(img_size);
            if (bkgnd_img == NULL) {
                status = -1;
                goto image_mode_test_error;
            }

            status = fps_switch_sensor_mode(device_handle, FPS_IMAGE_MODE, &mode_old);
            if (status < 0) {
                goto image_mode_test_error;
            }

            status = fps_get_averaged_image(device_handle,
                                            img_width,
                                            img_height,
                                            frms_to_avg,
                                            bkgnd_img,
                                            NULL, NULL, NULL);
            if (status < 0) {
                goto image_mode_test_error;
            }

            status = fps_switch_sensor_mode(device_handle, mode_old, NULL);
            if (status < 0) {
                goto image_mode_test_error;
            }

            status = fps_set_background_image(device_handle,
                                              img_width,
                                              img_height,
                                              bkgnd_img);
            if (status < 0) {
                goto image_mode_test_error;
            }

            free(bkgnd_img);

            printf("    Done!\n");
        }

        // Get Images
        // ---------------------------------------------------------------------

        if (cmd_key == '\n') {

            printf("    Getting Images...\n");

            // Create image buffers to store backgournd, foreground and finger-only images
            bkgnd_img = (uint8_t *) malloc(img_size);
            if (bkgnd_img == NULL) {
                status = -1;
                goto image_mode_test_error;
            }

            frgnd_img = (uint8_t *) malloc(img_size);
            if (frgnd_img == NULL) {
                status = -1;
                goto image_mode_test_error;
            }

            finger_img = (uint8_t *) malloc(img_size);
            if (finger_img == NULL) {
                status = -1;
                goto image_mode_test_error;
            }

            // Get background image and its average
            status = fps_get_background_image(device_handle,
                                              img_width,
                                              img_height,
                                              bkgnd_img);
            if (status < 0) {
                goto image_mode_test_error;
            }

            status = fps_get_background_average(device_handle, &bkgnd_avg);
            if (status < 0) {
                goto image_mode_test_error;
            }

            // Save background image
            sprintf(file_name, "%s/background.bmp", image_folder);
            status = save_bmp(file_name, bkgnd_img, img_size);
            if (status < 0) {
                goto image_mode_test_error;
            }

            // Save current settings
            sprintf(file_name, "%s/settings.txt", image_folder);
            fptr = fopen(file_name, "w");
            if (fptr == NULL) {
                return status;
            }

            addr[0] = FPS_REG_ANA_I_SET_0;
            addr[1] = FPS_REG_ANA_I_SET_1;
            addr[2] = FPS_REG_SET_ANA_0;
            addr[3] = FPS_REG_SET_ANA_1;
            addr[4] = FPS_REG_SET_ANA_2;

            status = fps_multiple_read(device_handle, addr, data, 5);
            if (status < 0) {
                return status;
            }

            fprintf(fptr, "Folder '%s' Settings:\n", image_folder);
            fprintf(fptr, "\n");
            fprintf(fptr, "Number of Frames to Average = %0d\n", frms_to_avg);
            fprintf(fptr, "\n");
            fprintf(fptr, "Background Average = %0.3f\n", bkgnd_avg);
            fprintf(fptr, "\n");
            fprintf(fptr, "CDS Offset 0 = 0x%02X\n", cds_offset_0);
            fprintf(fptr, "CDS Offset 1 = 0x%03X\n", cds_offset_1);
            fprintf(fptr, "PGA Gain   0 = 0x%02X\n", pga_gain_0);
            fprintf(fptr, "PGA Gain   1 = 0x%02X\n", pga_gain_1);
            fprintf(fptr, "\n");
            fprintf(fptr, "ANA_I_SET_0 (0x%02X) = 0x%02X\n", FPS_REG_ANA_I_SET_0, data[0]);
            fprintf(fptr, "ANA_I_SET_1 (0x%02X) = 0x%02X\n", FPS_REG_ANA_I_SET_1, data[1]);
            fprintf(fptr, "SET_ANA_0   (0x%02X) = 0x%02X\n", FPS_REG_SET_ANA_0,   data[2]);
            fprintf(fptr, "SET_ANA_1   (0x%02X) = 0x%02X\n", FPS_REG_SET_ANA_1,   data[3]);
            fprintf(fptr, "SET_ANA_2   (0x%02X) = 0x%02X\n", FPS_REG_SET_ANA_2,   data[4]);
            fprintf(fptr, "\n");
            fclose(fptr);

            stopwatch_start(&stopwatch);

            start_ctrl_c_monitor();

            status = fps_switch_sensor_mode(device_handle, FPS_IMAGE_MODE, &mode_old);
            if (status < 0) {
                goto image_mode_test_error;
            }

            // Repeat to acquire images
            for (n = 0; ((n < num_of_imgs) && (is_ctrl_c_hit() == FALSE)); n++) {

                // Get foreground image
                if (img_type == 0) {
                    status = fps_get_averaged_image(device_handle,
                                                    img_width,
                                                    img_height,
                                                    frms_to_avg,
                                                    frgnd_img,
                                                    &frgnd_avg,
                                                    NULL, NULL);
                    if (status < 0) {
                        goto image_mode_test_error;
                    }

                    // Calculate dynamic range
                    finger_dr = frgnd_avg - bkgnd_avg;
                    printf("    Image %03d DR = %0.3f\n", n, finger_dr);

                    // Save this image
                    sprintf(file_name, "%s/%03d_average.bmp", image_folder, n);
                    status = save_bmp(file_name, frgnd_img, img_size);
                    if (status < 0) {
                        goto image_mode_test_error;
                    }

                }

                // Get finger-only image
                else
                if (img_type == 1) {
                    status = fps_get_finger_image(device_handle,
                                                  img_width,
                                                  img_height,
                                                  frms_to_avg,
                                                  finger_img,
                                                  &finger_dr,
                                                  NULL, NULL);
                    if (status < 0) {
                        goto image_mode_test_error;
                    }

                    printf("    Image %03d DR = %0.3f\n", n, finger_dr);

                    // Save this image
                    sprintf(file_name, "%s/%03d_finger.bmp", image_folder, n);
                    status = save_bmp(file_name, finger_img, img_size);
                    if (status < 0) {
                        goto image_mode_test_error;
                    }
                }
            }

            stop_ctrl_c_monitor();

            stopwatch_stop(&stopwatch);
            elapsed = stopwatch_get_elapsed(&stopwatch);

            printf("    Done!\n");
            printf("\n");
            printf("    Time Elapsed = %0.3f ms\n", elapsed);

image_mode_test_error :

            // Free allocated buffers
            if (finger_img != NULL) {
                free(finger_img);
            }

            if (frgnd_img != NULL) {
                free(frgnd_img);
            }

            if (bkgnd_img != NULL) {
                free(bkgnd_img);
            }
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();
    }

    return status;
}
