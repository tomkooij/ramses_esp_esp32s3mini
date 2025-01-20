
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

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"

#include "cmd.h"
#include <ramses_buttons.h>
#include "ramses-mqtt.h"
#include "gateway.h"

#include "platform.h"
#include "host.h"

struct host_data {
  BaseType_t coreID;
  TaskHandle_t task;
  uint8_t platforms;
};

/*************************************************************************
 * TASK
 */

void host_button_cb( struct button_event *event ) {
  static TickType_t ticks = 0;
  static int8_t level = -1;

  if( event ) {
    if( event->level != level ) {
      switch( event->level ) {
      case 0:  // Button pressed
        // Nothing specific to do
    	break;

      default:	// Button released
    	if( level!=-1 ) {
          TickType_t period = (level==-1) ? 0
                                          : event->ticks - ticks;
          if( period > ( 1000 / portTICK_PERIOD_MS )) {
            printf("Restarting...\n");
            fflush(stdout);
      	    esp_restart();
          }
    	}
        break;
      }

      ticks = event->ticks;
      level = event->level;
    }
  }
}

static void Host_Task( void *param )
{
  struct host_data *ctxt = param;
  struct cmd_data *cmd_data;

  ESP_LOGI( TAG, "Task Started");

  // Basic console initialisation
  cmd_data = cmd_init();
  ramses_mqtt_init( ctxt->coreID );

  button_register( BUTTON_FUNC, host_button_cb );

  if( ctxt->platforms & PLATFORM_GW )
    gateway_init( ctxt->coreID );

  cmd_work( cmd_data );
}

/*************************************************************************
 * External API
 */

static struct host_data *host_context( void ) {
  static struct host_data host;

  static struct host_data *ctxt = NULL;
  if( !ctxt ){
	ctxt = &host;
	// Initialisation?
  }

  return ctxt;
}


void Host_init( BaseType_t coreID, uint8_t platforms ) {
  struct host_data *ctxt = host_context();

  ctxt->coreID = coreID;
  ctxt->platforms = platforms;

  xTaskCreatePinnedToCore( Host_Task,  "Host",  4096, ctxt, 10, &ctxt->task, ctxt->coreID );
}
