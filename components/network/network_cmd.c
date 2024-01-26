/********************************************************************
 * ramses_esp
 * network_cmd.h
 *
 * (C) 2023 Peter Price
 *
 * General Network Commands
 *
 */
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "esp_console.h"

#include "network.h"
#include "network_cmd.h"

/*********************************************************
 * Helper Functions
 */
static esp_console_cmd_func_t cmd_lookup( esp_console_cmd_t const *list, char *command ) {
  while( list->func != NULL ) {
    if( !strcmp( command, list->command ) )
      break;
    list++;
  }

  return list->func;
}

static void cmd_help( esp_console_cmd_t const *list, char *prefix ) {
  while( list->func ) {
	printf("%s %10s %s\n",prefix,list->command,list->help );
	list++;
  }
}

/*********************************************************
 * MQTT Server command
 */
static int net_cmd_mqtt_broker( int argc, char **argv ) {

  if( argc>1 )
	  NET_set_mqtt_broker( argv[1] );

  return 0;
}

/*********************************************************
 * SNTP Server command
 */
static int net_cmd_sntp_server( int argc, char **argv ) {

  if( argc>1 )
	  NET_set_sntp_server( argv[1] );

  return 0;
}

/*********************************************************
 * Top Level command
 */
enum net_cmd_list {
  NET_CMD_MQTT,
  NET_CMD_SNTP,
  // Last
  NET_CMD_MAX
};

static esp_console_cmd_t const cmds[NET_CMD_MAX+1] = {
  [NET_CMD_MQTT] = {
    .command = "mqtt",
    .help = "Set MQTT <Broker>",
    .hint = NULL,
    .func = &net_cmd_mqtt_broker,
  },
  [NET_CMD_SNTP] = {
    .command = "sntp",
    .help = "Set SNTP <Server>",
    .hint = NULL,
    .func = &net_cmd_sntp_server,
  },
  // List termination
  [NET_CMD_MAX] = {
    .command = NULL,
    .help = NULL,
    .hint = NULL,
    .func = NULL,
  },
};

static int net_cmd( int argc, char **argv ) {
  if( argc==1 ) {
	cmd_help( cmds, "net" );
  } else {
	esp_console_cmd_func_t func = cmd_lookup( cmds, argv[1] );
	if( func )
      ( func )( --argc, ++argv );
	else
      cmd_help( cmds, "net" );
  }

  return 0;
}

void network_register(void) {
  const esp_console_cmd_t network = {
    .command = "net",
    .help = "Netowrk commands, enter 'net' for list",
	.hint = NULL,
	.func = &net_cmd,
  };

  ESP_ERROR_CHECK( esp_console_cmd_register(&network) );
}
