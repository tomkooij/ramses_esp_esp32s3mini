/********************************************************************
 * ramses_esp
 * cmd.c
 *
 * (C) 2024 Peter Price
 *
 * General command handling
 *
 */

static const char *TAG = "CMD";
#include "esp_log.h"

#include "driver/usb_serial_jtag.h"
#include "esp_system.h"

#include "cmd.h"

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

static void cmd_help( esp_console_cmd_t const *list, char const *prefix ) {
  while( list->func ) {
	printf("# %s %-10s %s\n",prefix,list->command,list->help );
	list++;
  }
}

int cmd_menu( int argc, char **argv, esp_console_cmd_t const *cmds, char const *menu ) {
  if( argc==1 ) {
	cmd_help( cmds, menu );
  } else {
	esp_console_cmd_func_t func = cmd_lookup( cmds, argv[1] );
	if( func )
	  ( func )( --argc, ++argv );
	else
	  cmd_help( cmds, menu );
  }

  return 0;
}

void cmd_menu_register( esp_console_cmd_t const *cmd ) {
  while( cmd->command != NULL ) {
	ESP_ERROR_CHECK( esp_console_cmd_register(cmd) );
	cmd++;
  }
}

/*************************************************************************
 * Console input
 */

static void console_init( char *line, int len ) {
  usb_serial_jtag_driver_config_t config = {
    .rx_buffer_size = 1024,
    .tx_buffer_size = 1024
  };
  ESP_ERROR_CHECK( usb_serial_jtag_driver_install(&config) );

  // Flush the input buffer
  while( usb_serial_jtag_read_bytes( line, len, 0 ) ){}
}

static int console_readline( char *line, int len ) {
  int n = 0;

  do {
    int ret;
    do {
      // Read one char at a time so we don't have to remember
      // any characters from next line already in USB buffer
      ret = usb_serial_jtag_read_bytes( line+n, 1, 0 );
        if( ret>0 ) {
        if( line[n] == '\r' ) {
          if( n>0 ) {
            line[n]='\0';
            printf( "# %s\n", line );
            return n;
          }
        } else if( line[n] == '\b' ) {
          if( n>0 ) n=n-1;
        } else if( line[n] >= ' ' ) {    // Discard other non-printing characters
            n++;
        }
        }
    } while( ret );
    // USB buffer is empty
    vTaskDelay(10/portTICK_PERIOD_MS);
  } while(1);

  return -1;    // Cannot reach here
}

/*********************************************************
 * utility commands
 */

static int cmd_reset( int argc, char **argv ) {
  esp_restart();
  return 0;
}

/*********************************************************************/

static int cmd_cmd( int argc, char **argv );  // forward declaration

static esp_console_cmd_t const cmd_cmds[] = {
  {
    .command = "cmd",
    .help = "List utility commands",
    .hint = NULL,
    .func = &cmd_cmd,
  },
  {
    .command = "reset",
    .help = "Soft reset",
    .hint = NULL,
    .func = &cmd_reset,
  },
  // List termination
  {
    .command = NULL,
    .help = NULL,
    .hint = NULL,
    .func = NULL,
  }
};

static int cmd_cmd( int argc, char **argv ) {
  cmd_help( cmd_cmds, "" );
  return 0;
}

void cmd_register(void) {
  esp_console_cmd_t const *cmd;

  esp_console_config_t console_config = {
    .max_cmdline_args = 10,
    .max_cmdline_length = 256,
  };
  ESP_ERROR_CHECK( esp_console_init(&console_config) );

  esp_console_register_help_command();

  for( cmd=cmd_cmds ; cmd->command!=NULL ; cmd++ )
    ESP_ERROR_CHECK( esp_console_cmd_register( cmd ) );
}

/*********************************************************************/

esp_err_t cmd_run( char const *cmdline, int *cmd_ret ) {
  esp_err_t err = esp_console_run( cmdline, cmd_ret );
  if( err != ESP_OK ) {
	ESP_LOGW( TAG, "<%s> error %s <%d>",cmdline,esp_err_to_name(err),err );
  }
  return err;
}

static struct cmd_data {
  char line[256];
} cmd_data ;

struct cmd_data *cmd_init(void) {
  static struct cmd_data *data = &cmd_data;

  console_init( data->line, sizeof(data->line) );
  cmd_register();

  return data;
}


void cmd_work( struct cmd_data *data ) {
  for( ;; ) {
    int ret;
    esp_err_t err;

    console_readline( data->line, sizeof(data->line) );

    err = esp_console_run( data->line, &ret );
    if( err==ESP_ERR_NOT_FOUND ) {
      ESP_LOGI(TAG, "Unrecognized command <%s>",data->line );
    } else if( err==ESP_ERR_INVALID_ARG ) {
      // command was empty
    } else if( err==ESP_OK && ret!=ESP_OK) {
      ESP_LOGI(TAG, "Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
    } else if( err!=ESP_OK ) {
      ESP_LOGI(TAG, "Internal error: %s\n", esp_err_to_name(err));
    }
  }
}
