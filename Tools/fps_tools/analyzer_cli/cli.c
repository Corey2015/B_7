#include <stdio.h>
#include <string.h>
#include "common.h"

#if defined(__LINUX__)
    #include <signal.h>
    #include <sys/stat.h>
    #include <sys/types.h>
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Clear Console Buffer
//

void
clear_console()
{
#if !defined(__DEBUG__)
    printf("\033[H\033[J");
#endif
}


////////////////////////////////////////////////////////////////////////////////
//
// Get Commands from CLI
//

char_t
get_command(char *line,
            int  count)
{
    char_t key;

    memset(line, '\0', count);
    fgets(line, count, stdin);

    key = line[0];
    line[strlen(line) - 1] = '\0';

    return key;
}


////////////////////////////////////////////////////////////////////////////////
//
// Ctrl-C Manipulation
//

static int ctrl_c_hit = FALSE;

int
is_ctrl_c_hit()
{
    return (ctrl_c_hit == TRUE);
}

#if defined(__WINDOWS__)
BOOL WINAPI ctrl_c_handler(DWORD type)
{
    if (type == CTRL_C_EVENT) {
        ctrl_c_hit = TRUE;
    }

	return TRUE;
}

int
start_ctrl_c_monitor()
{
    int status = 0;

    ctrl_c_hit = FALSE;
    status = SetConsoleCtrlHandler((PHANDLER_ROUTINE) ctrl_c_handler, TRUE);
    if (status == TRUE) {
        return status = 0;
    } else {
        return status = -1;
    }
}

int
stop_ctrl_c_monitor()
{
    int status = 0;

    ctrl_c_hit = FALSE;
    status = SetConsoleCtrlHandler((PHANDLER_ROUTINE) ctrl_c_handler, FALSE);
    if (status == TRUE) {
        return status = 0;
    } else {
        return status = -1;
    }
}
#elif defined(__LINUX__)
static void ctrl_c_handler(int id)
{
    ctrl_c_hit = TRUE;
}

int
start_ctrl_c_monitor()
{
    ctrl_c_hit = FALSE;
    signal(SIGINT, ctrl_c_handler);
    return 0;

}

int
stop_ctrl_c_monitor()
{
    ctrl_c_hit = FALSE;
    signal(SIGINT, SIG_DFL);
    return 0;
}
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Create Folders
//

int
create_folder(char_t *path)
{
    int status = 0;

#if defined(__WINDOWS__)
    if (CreateDirectory(path, NULL) == FALSE) {
        status = -1;
    }
#elif defined(__LINUX__)
    status = mkdir(path, 0777);
#endif

    return status;
}


