/********************************************************************
 * ramses_esp
 * cmd.h
 *
 * (C) 2024 Peter Price
 *
 * General command handling
 *
 */
#ifndef _CMD_H_
#define _CMD_H_

#include <string.h>
#include <stdio.h>

#include "esp_err.h"
#include "esp_console.h"

extern esp_console_cmd_func_t cmd_lookup( esp_console_cmd_t const *list, char *command );
extern void cmd_help( esp_console_cmd_t const *list, char *prefix );

extern esp_err_t cmd_run( char const *cmdline, int *cmd_ret );

extern void cmd_init(void);

#endif // _CMD_H_
