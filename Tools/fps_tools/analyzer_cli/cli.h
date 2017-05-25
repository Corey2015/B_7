#ifndef __cli_h__
#define __cli_h__


#include "common.h"


#if defined(__cplusplus)
extern "C" {
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Prototypes
//

void clear_console();

char_t get_command(char *line,
                   int  count);

int is_ctrl_c_hit();

int start_ctrl_c_monitor();

int stop_ctrl_c_monitor();

int create_folder(char *path);


#if defined(__cplusplus)
}
#endif


#endif // __cli_h__
