/********************************************************************
 * ramses_esp
 * ota_cmd.c
 *
 * (C) 2023 Peter Price
 *
 * OTA Commands
 *
 */
#include "cmd.h"
#include "ota_cmd.h"
#include "ramses_ota.h"

/*********************************************************
 * ORL command
 */
static int ota_cmd_url( int argc, char **argv ) {

  if( argc>1 )
    ota_set_url( argv[1] );

  return 0;
}

/*********************************************************
 * Version command
 */
static int ota_cmd_version( int argc, char **argv ) {

  if( argc>1 )
    ota_set_version( argv[1] );

  return 0;
}

/*********************************************************
 * filename command
 */
static int ota_cmd_filename( int argc, char **argv ) {

  if( argc>1 )
	ota_set_filename( argv[1] );

  return 0;
}

/*********************************************************
 * Start command
 */
static int ota_cmd_start( int argc, char **argv ) {

  ota_start();

  return 0;
}

/*********************************************************
 * Top Level command
 */
enum ota_cmd_list {
  OTA_CMD_URL,
  OTA_CMD_VERSION,
  OTA_CMD_FILENAME,
  OTA_CMD_START,
  OTA_CMD_MAX
};

static esp_console_cmd_t const cmds[OTA_CMD_MAX+1] = {
  [OTA_CMD_URL] = {
    .command = "url",
    .help = "Set OTA <url>",
    .hint = NULL,
    .func = ota_cmd_url,
  },
  [OTA_CMD_VERSION] = {
    .command = "version",
    .help = "Set OTA <version>",
    .hint = NULL,
    .func = ota_cmd_version,
  },
  [OTA_CMD_FILENAME] = {
    .command = "filename",
    .help = "Set OTA <filename>",
    .hint = NULL,
    .func = ota_cmd_filename,
  },
  [OTA_CMD_START] = {
    .command = "start",
    .help = "Start OTA",
    .hint = NULL,
    .func = ota_cmd_start,
  },
  // List termination
  [OTA_CMD_MAX] = {
    .command = NULL,
    .help = NULL,
    .hint = NULL,
    .func = NULL,
  },
};

static int ota_cmd( int argc, char **argv ) {
  if( argc==1 ) {
	cmd_help( cmds, "ota" );
  } else {
	esp_console_cmd_func_t func = cmd_lookup( cmds, argv[1] );
	if( func )
      ( func )( --argc, ++argv );
	else
      cmd_help( cmds, "ota" );
  }

  return 0;
}

void ota_register(void) {
  const esp_console_cmd_t ota = {
    .command = "ota",
    .help = "OTA commands, enter 'ota' for list",
	.hint = NULL,
	.func = &ota_cmd,
  };

  ESP_ERROR_CHECK( esp_console_cmd_register(&ota) );
}
