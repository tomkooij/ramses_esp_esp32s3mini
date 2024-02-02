/********************************************************************
 * ramses_esp
 * network_cmd.h
 *
 * (C) 2023 Peter Price
 *
 * General Network Commands
 *
 */
#include "cmd.h"
#include "network_cmd.h"

#include "ramses_network.h"

/*********************************************************
 * MQTT Server commands
 */
enum mqtt_cmd_list {
  MQTT_CMD_BROKER,
  MQTT_CMD_USER,
  MQTT_CMD_PASSWORD,
  // Last
  MQTT_CMD_MAX
};

static int mqtt_cmd_broker( int argc, char **argv ) {

  if( argc>1 )
	  NET_set_mqtt_broker( argv[1] );

  return 0;
}

static int mqtt_cmd_user( int argc, char **argv ) {

  if( argc>1 )
	  NET_set_mqtt_user( argv[1] );

  return 0;
}

static int mqtt_cmd_password( int argc, char **argv ) {

  if( argc>1 )
	  NET_set_mqtt_password( argv[1] );

  return 0;
}

static esp_console_cmd_t const mqtt_cmds[MQTT_CMD_MAX+1] = {
  [MQTT_CMD_BROKER] = {
    .command = "broker",
    .help = "Set MQTT <Broker>",
    .hint = NULL,
    .func = &mqtt_cmd_broker,
  },
  [MQTT_CMD_USER] = {
    .command = "user",
    .help = "Set MQTT <User>",
    .hint = NULL,
    .func = &mqtt_cmd_user,
  },
  [MQTT_CMD_PASSWORD] = {
    .command = "password",
    .help = "Set MQTT <Password>",
    .hint = NULL,
    .func = &mqtt_cmd_password,
  },
  // List termination
  [MQTT_CMD_MAX] = {
    .command = NULL,
    .help = NULL,
    .hint = NULL,
    .func = NULL,
  },
};

static int net_cmd_mqtt( int argc, char **argv ) {

  if( argc==1 ) {
	cmd_help( mqtt_cmds, "mqtt" );
  } else {
	esp_console_cmd_func_t func = cmd_lookup( mqtt_cmds, argv[1] );
	if( func )
	  ( func )( --argc, ++argv );
	else
	  cmd_help( mqtt_cmds, "mqtt" );
  }

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
    .help = "Set MQTT parameters",
    .hint = NULL,
    .func = &net_cmd_mqtt,
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
