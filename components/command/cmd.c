/********************************************************************
 * ramses_esp
 * cmd.c
 *
 * (C) 2024 Peter Price
 *
 * General command handling
 *
 */

#include "cmd.h"

/*********************************************************
 * Helper Functions
 */
esp_console_cmd_func_t cmd_lookup( esp_console_cmd_t const *list, char *command ) {
  while( list->func != NULL ) {
    if( !strcmp( command, list->command ) )
      break;
    list++;
  }

  return list->func;
}

void cmd_help( esp_console_cmd_t const *list, char *prefix ) {
  while( list->func ) {
	printf("%s %10s %s\n",prefix,list->command,list->help );
	list++;
  }
}

/*********************************************************
 * Console initialisation
 */

void console_init(void) {
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  /* Prompt to be printed before each line.
   * This can be customized, made dynamic, etc.
   */
  repl_config.prompt = "";
  repl_config.max_cmdline_length = 1024;

  esp_console_register_help_command();
//  register_test();

#if defined(CONFIG_ESP_CONSOLE_UART_DEFAULT) || defined(CONFIG_ESP_CONSOLE_UART_CUSTOM)
    esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

#elif defined(CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG)
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));

#else
#error Unsupported console type
#endif

  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}

