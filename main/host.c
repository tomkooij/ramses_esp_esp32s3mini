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
#include "cmd.h"

#include "ramses-mqtt.h"
#include "gateway.h"
#include "host.h"

struct host_data {
  BaseType_t coreID;
  TaskHandle_t task;
};

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
