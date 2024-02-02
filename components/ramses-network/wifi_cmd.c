/********************************************************************
 * ramses_esp
 * wifi_cmd.c
 *
 * (C) 2023 Peter Price
 *
 * WiFi Commands
 *
 */
#include "cmd.h"
#include "wifi_cmd.h"
#include "ramses_wifi.h"

/*********************************************************
 * SSID command
 */
static int wifi_cmd_ssid( int argc, char **argv ) {

  if( argc>1 )
    wifi_set_ssid( argv[1] );

  return 0;
}

/*********************************************************
 * Password command
 */
static int wifi_cmd_password( int argc, char **argv ) {

  if( argc>1 )
    wifi_set_password( argv[1] );

  return 0;
}

/*********************************************************
 * Restart command
 */
static int wifi_cmd_restart( int argc, char **argv ) {

  wifi_restart();

  return 0;
}

/*********************************************************
 * Status command
 */
/*
static int wifi_cmd_status( int argc, char **argv ) {

  wifi_status();

  return 0;
}
*/
/*********************************************************
 * Top Level command
 */
enum wifi_cmd_list {
  WIFI_CMD_SSID,
  WIFI_CMD_PASSWORD,
  WIFI_CMD_RESTART,
//  WIFI_CMD_STATUS,
  WIFI_CMD_MAX
};

static esp_console_cmd_t const cmds[WIFI_CMD_MAX+1] = {
  [WIFI_CMD_SSID] = {
    .command = "ssid",
    .help = "Set WiFi <SSID>",
    .hint = NULL,
    .func = &wifi_cmd_ssid,
  },
  [WIFI_CMD_PASSWORD] = {
    .command = "password",
    .help = "Set WiFi <PASSWORD>",
    .hint = NULL,
    .func = &wifi_cmd_password,
  },
  [WIFI_CMD_RESTART] = {
    .command = "restart",
    .help = "Restart WiFi",
    .hint = NULL,
    .func = &wifi_cmd_restart,
  },
/*
  [WIFI_CMD_STATUS] = {
    .command = "status",
    .help = "Show WiFi status",
    .hint = NULL,
    .func = &wifi_cmd_status,
  },
*/
  // List termination
  [WIFI_CMD_MAX] = {
    .command = NULL,
    .help = NULL,
    .hint = NULL,
    .func = NULL,
  },
};

static int wifi_cmd( int argc, char **argv ) {
  if( argc==1 ) {
	cmd_help( cmds, "wifi" );
  } else {
	esp_console_cmd_func_t func = cmd_lookup( cmds, argv[1] );
	if( func )
      ( func )( --argc, ++argv );
	else
      cmd_help( cmds, "wifi" );
  }

  return 0;
}

void wifi_register(void) {
  const esp_console_cmd_t wifi = {
    .command = "wifi",
    .help = "WiFi commands, enter 'wifi' for list",
	.hint = NULL,
	.func = &wifi_cmd,
  };

  ESP_ERROR_CHECK( esp_console_cmd_register(&wifi) );
}
