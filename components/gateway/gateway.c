/********************************************************************
 * ramses_esp
 * gateway.c
 *
 * (C) 2023 Peter Price
 *
 * The Gateway host interface implements the emulation of an HGI80
 * that is directly connected to the host via a USB port.
 *
 * RX messages are printed in the HGI80 format
 * TX messages are accepted from the host and forwarded to the Host task
 *
 */
static const char *TAG = "GATEWAY";
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_console.h"
#include "gateway.h"

struct gateway_data {
  BaseType_t coreID;
  TaskHandle_t   task;
  QueueHandle_t  queue;
};

static struct gateway_data *gateway_ctxt( void ) {
  static struct gateway_data gateway;

  struct gateway_data *ctxt = NULL;
  if( !ctxt ){
    ctxt = &gateway;
    // Initialisation?
  }

  return ctxt;
}

/*************************************************************************
 * TASK
 */

struct gateway_msg {
  void (* msgFunc)( void *param );
  void *param;
};

static BaseType_t gateway_msg_post( struct gateway_msg * msg ) {
  BaseType_t res = pdFALSE;

  struct gateway_data *ctxt = gateway_ctxt();
  if( ctxt && ctxt->queue )
    res = xQueueSend( ctxt->queue, msg, portTICK_PERIOD_MS );

  return res;
}

static void Gateway( void *param ) {
  struct gateway_data *ctxt = param;

  ESP_LOGI( TAG, "Task Started");

  ctxt->queue = xQueueCreate( 10, sizeof( struct gateway_msg ) );

  do {
    struct gateway_msg msg;
    BaseType_t res = xQueueReceive( ctxt->queue, &msg, portTICK_PERIOD_MS);
    if( res ){
      if( msg.msgFunc )
        ( msg.msgFunc )( msg.param );
    }
  } while(1);
}

/*************************************************************************
 * Actions
 */

static void gateway_radio_rx_func( void *param ) {
  struct message *msg = param;
  ESP_LOGI( TAG, "process rx message %p",msg);

  if( msg ) {
	char msgBuff[256];
    uint8_t len = msg_print_all( msg, msgBuff );
    if( len )
      printf(msgBuff);
    msg_free( &msg );
  }
}

void gateway_radio_rx( struct message **message ) {
  struct gateway_msg msg = {
    .msgFunc = gateway_radio_rx_func,
	.param = *message
  };

  BaseType_t res = gateway_msg_post( &msg );
  if( !res ) {
	ESP_LOGE( TAG, "failed to post rx message");
	msg_free( message );
  } else {
	*message = NULL;  // We own message now
  }
}

/*************************************************************************
 * TX messages
 */
static int gateway_radio_tx(int argc, char **argv) {
  if( argc==8 ) {
	char cmd[256];
    sprintf( cmd, "%s %s %s %s %s %s %s %s",
    		argv[0],argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7]);

	ESP_LOGI( TAG, "%s",  cmd );
  } else {
	ESP_LOGI(TAG, "discarded %s + %d parameters",argv[0],argc-1);
  }
  return 0;
}

static void gateway_register_tx(void) {
    const esp_console_cmd_t i = {
        .command = "I",
        .help = "Send I message",
        .hint = NULL,
        .func = &gateway_radio_tx,
    };
    const esp_console_cmd_t w = {
        .command = "W",
        .help = "Send W message",
        .hint = NULL,
        .func = &gateway_radio_tx,
    };
    const esp_console_cmd_t rq = {
        .command = "RQ",
        .help = "Send RQ message",
        .hint = NULL,
        .func = &gateway_radio_tx,
    };
    const esp_console_cmd_t rp = {
        .command = "I",
        .help = "Send RP message",
        .hint = NULL,
        .func = &gateway_radio_tx,
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&i ) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&w ) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&rq) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&rp) );
}

/*************************************************************************
 * External API
 */

void gateway_init( BaseType_t coreID ) {
  struct gateway_data *ctxt = gateway_ctxt();
  ctxt->coreID = coreID;

  gateway_register_tx();

  xTaskCreatePinnedToCore( Gateway, "Gateway", 4096, ctxt, 10, &ctxt->task, ctxt->coreID );
}
