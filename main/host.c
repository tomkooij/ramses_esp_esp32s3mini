/********************************************************************
 * ramses_esp
 * host.c
 *
 * (C) 2023 Peter Price
 *
 * Host interface
 * Common interface between Radio and supported communication protocols
 *
 * Keeps track of active protocol(s) and manages release of message resources
 *
 */
static const char *TAG = "HOST";
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_console.h"

#include "ramses-mqtt.h"
#include "gateway.h"
#include "host.h"

struct host_data {
  BaseType_t coreID;
  TaskHandle_t task;
};

static void console_init(void) {
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

/*************************************************************************
 * TASK
 */
#define CMD '!'

struct tty_ctxt {
  char buf[256];
  bool cmd;
};

static void tty_work(struct tty_ctxt *ctxt) {

}

static void Host_Task( void *param )
{
  struct host_data *ctxt = param;

  ESP_LOGI( TAG, "Task Started");

  // Basic console initialisation
  console_init();
  ramses_mqtt_init( ctxt->coreID );

  gateway_init( ctxt->coreID );

  while(1){
//    ESP_LOGI( TAG, "Loop");
    vTaskDelay(1000/ portTICK_PERIOD_MS );
  }
}

/*************************************************************************
 * External API
 */

static struct host_data *host_context( void ) {
  static struct host_data host;

  struct host_data *ctxt = NULL;
  if( !ctxt ){
	ctxt = &host;
	// Initialisation?
  }

  return ctxt;
}


void Host_init( BaseType_t coreID ) {
  struct host_data *ctxt = host_context();

  ctxt->coreID = coreID;

  xTaskCreatePinnedToCore( Host_Task,  "Host",  4096, ctxt, 10, &ctxt->task, ctxt->coreID );
}
