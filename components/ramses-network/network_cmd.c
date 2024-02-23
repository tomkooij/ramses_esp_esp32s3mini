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

static esp_console_cmd_t const mqtt_cmds[] = {
  {
    .command = "broker",
    .help = "Set MQTT <Broker>",
    .hint = NULL,
    .func = &mqtt_cmd_broker,
  },
  {
    .command = "user",
    .help = "Set MQTT <User>",
    .hint = NULL,
    .func = &mqtt_cmd_user,
  },
  {
    .command = "password",
    .help = "Set MQTT <Password>",
    .hint = NULL,
    .func = &mqtt_cmd_password,
  },
  // List termination
  { NULL_COMMAND }
};

static int mqtt_cmd( int argc, char **argv ) {
  return cmd_menu( argc, argv, mqtt_cmds, argv[0] );
}

/*********************************************************
 * SNTP Server commands
 */

static int sntp_cmd_server( int argc, char **argv ) {

  if( argc>1 )
	  NET_set_sntp_server( argv[1] );

  return 0;
}

static esp_console_cmd_t const sntp_cmds[] = {
  {
    .command = "server",
    .help = "Set SNTP <Server>",
    .hint = NULL,
    .func = &sntp_cmd_server,
  },
  // List termination
  { NULL_COMMAND }
};

static int sntp_cmd( int argc, char **argv ) {
  return cmd_menu( argc, argv, sntp_cmds, argv[0] );
}

/*********************************************************
 * Timezone
 */

static int timezone_cmd( int argc, char **argv ) {

  if( argc>1 )
    NET_set_timezone( argv[1] );
  else
	NET_show_timezones();

  return 0;
}

/*********************************************************
 * Top Level commands
 */

void network_register(void) {
  static esp_console_cmd_t const net_cmds[] = {
    {
      .command = "mqtt",
      .help = "MQTT commands, enter 'mqtt' for list",
      .hint = NULL,
      .func = &mqtt_cmd,
    },
    {
      .command = "sntp",
      .help = "SNTP commands, enter 'sntp' for list",
      .hint = NULL,
      .func = &sntp_cmd,
    },
    {
      .command = "timezone",
      .help = "timezone <tz string>",
      .hint = NULL,
      .func = &timezone_cmd,
    },
    // List termination
    { NULL_COMMAND }
  };

  cmd_menu_register( net_cmds );
}
