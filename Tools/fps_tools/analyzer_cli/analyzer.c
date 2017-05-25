#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include "debug.h"
#include "fps.h"
#include "fps_register.h"
#include "fps_control.h"
#include "cli.h"
#include "sleep.h"


extern int image_calibration();
extern int image_mode_test();
extern int detect_calibration();
extern int detect_mode_test();

#if defined(__WINDOWS__)
static char device_path[MAX_STRING_LENGTH] = "\\\\.\\COM20";
#elif defined(__LINUX__)
static char device_path[MAX_STRING_LENGTH] = "/dev/dfs0";
#endif

fps_handle_t *device_handle = NULL;
char         image_folder[MAX_STRING_LENGTH];


////////////////////////////////////////////////////////////////////////////////
//
// Open Sensor
//

int
open_sensor()
{
    int status = 0;

    while (1) {

        clear_console();

        printf("\n");
        printf("=============\n");
        printf(" Open Sensor \n");
        printf("=============\n");
        printf("\n");
        printf("Result:\n");
        printf("\n");
        printf("    Opening sensor...\n");

        device_handle = fps_open_sensor(device_path);

        if (device_handle == NULL) {
            printf("    Failed!\n");
        } else {
            printf("    Successful!\n");
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();

        break;
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Close Sensor
//

int
close_sensor()
{
    int status = 0;

    while (1) {

        clear_console();

        printf("\n");
        printf("==============\n");
        printf(" Close Sensor \n");
        printf("==============\n");
        printf("\n");
        printf("Result:\n");
        printf("\n");
        printf("    Closing sensor...\n");

        status = fps_close_sensor(&device_handle);

        if (status < 0) {
            printf("    Failed!\n");
        } else {
            printf("    Successful!\n");
        }

        device_handle = 0;

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();

        break;
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Reset Sensor
//

int
reset_sensor()
{
    const int delay = 50 * 1000;

    int status = 0;

    while (1) {

        clear_console();

        printf("\n");
        printf("==============\n");
        printf(" Reset Sensor \n");
        printf("==============\n");
        printf("\n");
        printf("Result:\n");
        printf("\n");
        printf("    Resetting sensor...\n");

        status = fps_reset_sensor(device_handle, 0);
        sleep_us(delay);
        status = fps_reset_sensor(device_handle, 1);

        if (status < 0) {
            printf("    Failed!\n");
        } else {
            printf("    Successful!\n");
        }

        printf("\n");
        printf("Press ENTER cmd_key to continue... ");
        (void) getchar();

        break;
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Sensor Initialization
//

int
init_sensor()
{
    int  status = 0;
    char cmd_line[MAX_STRING_LENGTH];
    char cmd_key;
    char *cmd_opt;
    int  config;
    char descr[MAX_STRING_LENGTH];

    status = fps_get_power_config(device_handle, &config);
    if (status < 0) {
        return status;
    }

    while (1) {

        clear_console();

        printf("\n");
        printf("===================\n");
        printf(" Initialize Sensor \n");
        printf("===================\n");
        printf("\n");
        printf("    'p' <config> - Specify where the power configuration.                \n");
        printf("                   <config> = Power configuration identifiers.           \n");
        printf("                     0: 1.8V + 3.3V                                      \n");
        printf("                     1: 1.8V + 2.8V                                      \n");
        printf("                     2: 1.8V only                                        \n");
        printf("                     3: External                                         \n");
        printf("                   (This number must be decimal.)                        \n");
        printf("                                                                         \n");
        printf("    'g'          - Get the power configuration setting.                  \n");
        printf("                                                                         \n");
        printf("    ENTER        - Do the initialization.                                \n");
        printf("                                                                         \n");
        printf("    'q'          - Back to main menu.                                    \n");
        printf("\n");
        printf("Pleae enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("pgq\n", cmd_key) == NULL) {
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

        if (cmd_key == 'p') {
            cmd_opt = strtok(cmd_line, " ");
            cmd_opt = strtok(NULL, " ");
            config  = strtol(cmd_opt, NULL, 10);

            if ((config != FPS_POWER_CONFIG_1V8_3V3 ) &&
                (config != FPS_POWER_CONFIG_1V8_2V8 ) &&
                (config != FPS_POWER_CONFIG_1V8_ONLY) &&
                (config != FPS_POWER_CONFIG_EXTERNAL)) {
                printf("    ERROR: Power config must be >= %0d and <= %0d!\n",
                       FPS_POWER_CONFIG_1V8_3V3, FPS_POWER_CONFIG_EXTERNAL);

                status = fps_get_power_config(device_handle, &config);
                if (status < 0) {
                    return status;
                }

                sleep_ms(1000);
                continue;
            }

            goto init_sensor_show_options;
        }

        if (cmd_key == 'g') {

init_sensor_show_options :

            switch (config) {
                case FPS_POWER_CONFIG_1V8_3V3  : strcpy(descr, "1.8V + 3.3V"); break;
                case FPS_POWER_CONFIG_1V8_2V8  : strcpy(descr, "1.8V + 2.8V"); break;
                case FPS_POWER_CONFIG_1V8_ONLY : strcpy(descr, "1.8V onlyV");  break;
                case FPS_POWER_CONFIG_EXTERNAL : strcpy(descr, "External");    break;
                default                        : strcpy(descr, "Unknown");     break;
            }

            printf("    Power Source = %0d (%s)\n", config, descr);
        }

        if (cmd_key == '\n') {

            printf("    Initializing...\n");

            status = fps_set_power_config(device_handle, config);
            status = fps_init_sensor(device_handle);

            if (status < 0) {
                printf("    Failed!\n");
            } else {
                printf("    Successful!\n");
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
// Set/Get Register(s)
//

int
access_registers()
{
    int     status;
    char    cmd_line[MAX_STRING_LENGTH];
    char    cmd_key;
    char    *cmd_opt;
    char    last_cmd_line[MAX_STRING_LENGTH];
    uint8_t addr[FPS_REG_COUNT + 1];
    uint8_t data[FPS_REG_COUNT + 1];
    int     length;
    int     i;

    for (i = 0; i <= FPS_REG_COUNT; i++) {
        addr[i] = 0xFF;
        data[i] = 0x00;
    }

    while (1) {

        clear_console();

        printf("\n");
        printf("====================\n");
        printf(" Access Register(s) \n");
        printf("====================\n");
        printf("\n");
        printf("    's' <a1 d1 ... aN dN> - Write register a1 to aN one by one.          \n");
        printf("                              <a1 ... aN> = Register addresses to write. \n");
        printf("                              <d1 ... dN> = Register data to write.      \n");
        printf("                            (These arguments must be seperated by spaces \n");
        printf("                             and must be hexadecimal.)                   \n");
        printf("                                                                         \n");
        printf("    'g' <a1 a2 ... aN>    - Read register a1 to aN one by one.           \n");
        printf("                              <a1 ... aN> = Register addresses to read.  \n");
        printf("                            (These address must be seperated by spaces   \n");
        printf("                             and must be hexadecimal.)                   \n");
        printf("                                                                         \n");
        printf("    'd'                   - Dump all registers.                          \n");
        printf("                                                                         \n");
        printf("    ENTER                 - Repeat the last action again.                \n");
        printf("                                                                         \n");
        printf("    'q'                   - Back to main menu.                           \n");
        printf("\n");
        printf("Pleae enter: ");

        // Backup the last command line for repeat commands
        strncpy(last_cmd_line, cmd_line, strlen(cmd_line));

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("sgdq\n", cmd_key) == NULL) {
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

access_registers_again :

        if (cmd_key == 's') {
            for (i = 0; i < FPS_REG_COUNT; i++) {
                addr[i] = 0xFF;
                data[i] = 0x00;
            }

            cmd_opt = strtok(cmd_line, " ");
            for (i = 0; i < FPS_REG_COUNT; i++) {
                cmd_opt = strtok(NULL, " ");
                if (cmd_opt == NULL) {
                    break;
                }
                addr[i] = (uint8_t) strtol(cmd_opt, NULL, 16);

                cmd_opt = strtok(NULL, " ");
                if (cmd_opt == NULL) {
                    break;
                }
                data[i] = (uint8_t) strtol(cmd_opt, NULL, 16);
            }

            length = i;

            status = fps_multiple_write(device_handle, addr, data, length);
            if (status < 0) {
                return status;
            }
        }

        if (cmd_key == 'g') {
            for (i = 0; i < FPS_REG_COUNT; i++) {
                addr[i] = 0xFF;
            }

            cmd_opt = strtok(cmd_line, " ");
            for (i = 0; i < FPS_REG_COUNT; i++) {
                cmd_opt = strtok(NULL, " ");
                if (cmd_opt == NULL) {
                    break;
                }
                addr[i] = (uint8_t) strtol(cmd_opt, NULL, 16);
            }

            length = i;

            status = fps_multiple_read(device_handle, addr, data, length);
            if (status < 0) {
                return status;
            }
        }

        if (cmd_key == 'd') {
            for (i = 0; i < FPS_REG_COUNT; i++) {
                addr[i] = i;
            }

            length = i;

            status = fps_multiple_read(device_handle, addr, data, length);
            if (status < 0) {
                return status;
            }
        }

        if (cmd_key == '\n') {
            strncpy(cmd_line, last_cmd_line, strlen(cmd_line));
            cmd_key = cmd_line[0];
            goto access_registers_again;
        }

        for (i = 0; addr[i] != 0xFF; i++) {
            printf("    [ 0x%02X ] = 0x%02X\n", addr[i], data[i]);
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Power Down Mode Test
//

int
power_down_mode_test()
{
    int  status = 0;
    char cmd_line[MAX_STRING_LENGTH];
    char cmd_key;
    int  mode_old;

    while (1) {

        clear_console();

        printf("\n");
        printf("======================\n");
        printf(" Power Down Mode Test \n");
        printf("======================\n");
        printf("\n");
        printf("    ENTER              - Do Power Down Mode test.                        \n");
        printf("                                                                         \n");
        printf("    CTRL+C             - Exit the test loop.                             \n");
        printf("                                                                         \n");
        printf("    'q'                - Back to main menu.                              \n");
        printf("\n");
        printf("Pleae enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("q\n", cmd_key) == NULL) {
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

        if (cmd_key == '\n') {
            printf("    Testing Power Down...\n");

            // Switch to Power Down Mode
            status = fps_switch_sensor_mode(device_handle, FPS_POWER_DOWN_MODE, &mode_old);
            if (status < 0) {
                return status;
            }

            start_ctrl_c_monitor();

            while (is_ctrl_c_hit() == FALSE);

            stop_ctrl_c_monitor();

            printf("    Done!\n");
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Set/Get Debug Level
//

int
access_debug_level()
{
    int  status = 0;
    char cmd_line[MAX_STRING_LENGTH];
    char cmd_key;
    char *cmd_opt;
    int  level;

    status = get_debug_level(&level);
    if (status < 0) {
        return status;
    }

    while (1) {

        clear_console();

        printf("\n");
        printf("=====================\n");
        printf(" Set/Get Debug Level \n");
        printf("=====================\n");
        printf("\n");
        printf("    's' <level> - Set debug level.                                       \n");
        printf("                    <level> = debug level.                               \n");
        printf("                      0: Error.                                          \n");
        printf("                      1: Error, Warning.                                 \n");
        printf("                      2: Error, Warning, Info.                           \n");
        printf("                      3: Error, Warning, Info, Debug.                    \n");
        printf("                      4: Error, Warning, Info, Debug, Register Access.   \n");
        printf("                                                                         \n");
        printf("    'g'         - Get current specified debug level.                     \n");
        printf("                                                                         \n");
        printf("    'q'         - Back to main menu.                                     \n");
        printf("\n");
        printf("Pleae enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("sgq\n", cmd_key) == NULL) {
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

        if (cmd_key == 's') {
            cmd_opt = strtok(cmd_line, " ");
            cmd_opt = strtok(NULL, " ");
            level   = strtol(cmd_opt, NULL, 10);

            if ((level != LOG_LEVEL_ERROR ) &&
                (level != LOG_LEVEL_WARN  ) &&
                (level != LOG_LEVEL_INFO  ) &&
                (level != LOG_LEVEL_DEBUG ) &&
                (level != LOG_LEVEL_DETAIL)) {
                printf("    ERROR: Debug level must be >= %0d and <= %0d\n",
                       LOG_LEVEL_ERROR, LOG_LEVEL_DETAIL);

                status = get_debug_level(&level);
                if (status < 0) {
                    return status;
                }

                sleep_ms(1000);
                continue;
            }

            status = set_debug_level(level);
            if (status < 0) {
                printf("    ERROR: set_debug_level() failed!\n");
                return status;
            }

            goto access_debug_level_show_options;
        }

        if (cmd_key == 'g') {

access_debug_level_show_options :

            printf("    Debug Level = %0d\n", level);
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Set/Get Device Path
//

int
access_device_path()
{
    int  status = 0;
    char cmd_line[MAX_STRING_LENGTH];
    char cmd_key;
    char *cmd_opt;

    while (1) {

        clear_console();

        printf("\n");
        printf("=====================\n");
        printf(" Set/Get Device Path \n");
        printf("=====================\n");
        printf("\n");
        printf("    's' <path> - Set device path.                                        \n");
        printf("                   <path> = Device path.                                 \n");
        printf("                                                                         \n");
        printf("    'g'        - Get current specified device path.                      \n");
        printf("                                                                         \n");
        printf("    'q'        - Back to main menu.                                      \n");
        printf("\n");
        printf("Pleae enter: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (strchr("sgq\n", cmd_key) == NULL) {
            printf("    ERROR: Invalid command!\n");
            sleep_ms(1000);
            continue;
        }

        if (cmd_key == 'q') {
            break;
        }

        if (cmd_key == 's') {
            cmd_opt = strtok(cmd_line, " ");
            cmd_opt = strtok(NULL, " ");

            strcpy(device_path, cmd_opt);

            goto access_device_path_show_options;
        }

        if (cmd_key == 'g') {

access_device_path_show_options :

            printf("\n");
            printf("Result:\n");
            printf("\n");

            printf("    Device Path = %s\n", device_path);
        }

        printf("\n");
        printf("Press ENTER key to continue... ");
        (void) getchar();
    }

    return status;
}


////////////////////////////////////////////////////////////////////////////////
//
// Get Program Version
//

int
get_program_version()
{
    clear_console();

    printf("\n");
    printf("=====================\n");
    printf(" Get Program Version \n");
    printf("=====================\n");
    printf("\n");
    printf("Result:\n");
    printf("\n");

    printf("    Built at %s, %s.\n", __TIME__, __DATE__);

    printf("\n");
    printf("Press ENTER key to continue... ");
    (void) getchar();

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
// Quit This Program
//

int
quit_program()
{
    printf("\n");
    printf("Bye...\n");
    printf("\n");

    exit(0);
}


////////////////////////////////////////////////////////////////////////////////
//
// Main Program
//

static void exit_program_callback(void)
{
    (void) fps_close_sensor(&device_handle);
}

struct __menu_item {
    char command;
    char *description;
    int  (*function) ();
    int  require_open;
}
menu[] = {
    //   command   description                function                require_open
    // +---------+--------------------------+-----------------------+--------------+
    {    'o',      "Open Sensor",             open_sensor,            0            },
    {    'c',      "Close Sensor",            close_sensor,           1            },
    {    'r',      "Reset the sensor",        reset_sensor,           1            },
    {    'i',      "Sensor Initialization",   init_sensor,            1            },
    {    'R',      "Access Registers",        access_registers,       1            },
    {    'k',      "Image Calibration",       image_calibration,      1            },
    {    'K',      "Detect Calibration",      detect_calibration,     1            },
    {    't',      "Image Mode Test",         image_mode_test,        1            },
    {    'T',      "Detect Mode Test",        detect_mode_test,       1            },
    {    'p',      "Power-Down Mode Test",    power_down_mode_test,   1            },
    {    'l',      "Set Debug Level",         access_debug_level,     0            },
    {    'P',      "Set Device Path",         access_device_path,     0            },
    {    'v',      "Get Program Version",     get_program_version,    0            },
    {    'q',      "Quit",                    quit_program,           0            },
    // TODO: adding mordescription e functions here...
    { 0, NULL, NULL, 0 }
};

int
main(int argc, char *argv[])
{
	int       status = 0;
    time_t    test_time;
    struct tm *tm_ptr;
    char      cmd_line[MAX_STRING_LENGTH];
    char      cmd_key;
    int       i;

    atexit(exit_program_callback);

    // Create an directory to save images
    time(&test_time);
    tm_ptr = localtime(&test_time);
    sprintf(image_folder, "%04d%02d%02d_%02d%02d%02d",
            (tm_ptr->tm_year + 1900), (tm_ptr->tm_mon + 1), tm_ptr->tm_mday,
            tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);

    status = create_folder(image_folder);
    if (status < 0) {
        exit(-1);
    }

    while (1) {
        clear_console();

        printf("\n");
        printf("========================\n");
        printf(" FPS Analyzer Main Menu \n");
        printf("========================\n");
        printf("\n");

        for (i = 0; menu[i].command != '\0'; i++) {
            printf("    \'%c\' - %s\n", menu[i].command, menu[i].description);
        }

        printf("\n");
        printf("Pleae select: ");

        cmd_key = get_command(cmd_line, sizeof(cmd_line));

        if (cmd_key == '\n') {
            continue;
        }

        for (i = 0; menu[i].command != '\0'; i++) {
            if (cmd_key == menu[i].command) {
                if ((device_handle == 0) && (menu[i].require_open == 1)) {
                    printf("\n");
                    printf("    ERROR: Sensor should be opened first!\n");
                    printf("\n");
                    sleep_ms(1000);
                    break;
                }

                menu[i].function();
                break;
            }
        }
    }

    exit(0);
}

