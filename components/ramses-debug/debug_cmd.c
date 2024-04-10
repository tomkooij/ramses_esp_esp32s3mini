/********************************************************************
 * ramses_esp
 * debug_cmd.c
 *
 * (C) 2023 Peter Price
 *
 * Log filter Commands
 *
 */

//static const char *TAG = "DEBUG";
#include "esp_log.h"

#include "cmd.h"

/*********************************************************
 * Log level control
 */
static esp_log_level_t get_log_level( char const *level ) {
  static char const *name[] = {
    [ESP_LOG_NONE] = "none",
	[ESP_LOG_ERROR] = "error",
    [ESP_LOG_WARN] = "warn",
    [ESP_LOG_INFO] = "info",
    [ESP_LOG_DEBUG] = "debug",
    [ESP_LOG_VERBOSE] = "verbose"
  };
  esp_log_level_t l, result = ESP_LOG_NONE;
  for( l=ESP_LOG_NONE ; l<=ESP_LOG_VERBOSE ;  l++ ) {
    if( !strcmp(level,name[l]) ) {
      result = l;
      break;
    }
  }

  return result;
}

static int debug_cmd_master( int argc, char **argv ) {

  if( argc==2 ) {
    esp_log_level_t level = get_log_level( argv[1] );
    esp_log_set_level_master( level );
  }

  return 0;
}

static int debug_cmd_filter( int argc, char **argv ) {

  if( argc==3 ) {
    esp_log_level_t level = get_log_level( argv[2] );
    esp_log_level_set( argv[1], level );
  }

  return 0;
}

/*********************************************************
 * Top Level command
 */
static esp_console_cmd_t const debug_cmds[] = {
  {
    .command = "master",
    .help = "master <none|error|warn|info|debug|verbose>",
    .hint = NULL,
    .func = debug_cmd_master,
  },
  {
    .command = "filter",
    .help = "filter <tag> <none|error|warn|info|debug|verbose>",
    .hint = NULL,
    .func = debug_cmd_filter,
  },
  // List termination
  { NULL_COMMAND }
};

static int debug_cmd( int argc, char **argv ) {
  return cmd_menu( argc, argv, debug_cmds, argv[0] );
}

void debug_register(void) {
  const esp_console_cmd_t debug[] = {
    {
      .command = "debug",
      .help = "Debug commands, enter 'debug' for list",
	  .hint = NULL,
	  .func = &debug_cmd,
    },
	{ NULL_COMMAND }
  };

  cmd_menu_register( debug );
}
