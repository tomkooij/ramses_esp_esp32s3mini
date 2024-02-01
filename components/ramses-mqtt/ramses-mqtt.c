/********************************************************************
 * ramses_esp
 * ramses-mqtt.c
 *
 * (C) 2023 Peter Price
 *
 * MQTT Client
 *
 */
#include <string.h>

static const char *TAG = "MQTT";
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_app_desc.h"

#include "mqtt_client.h"

#include "device.h"
#include "ramses_wifi.h"
#include "ramses-mqtt.h"

/****************************************************
 * States
 */
#define MQTT_STATE_LIST \
  MQTT_STATE( MQTT_IDLE,	      "Idle" ) \
  MQTT_STATE( MQTT_WAIT,          "Wait" ) \
  MQTT_STATE( MQTT_START,         "Start" ) \
  MQTT_STATE( MQTT_STARTING,      "Starting" ) \
  MQTT_STATE( MQTT_CONNECTED,     "Connected" ) \
  MQTT_STATE( MQTT_ACTIVE,        "Active" ) \

#define MQTT_STATE( _e, _t ) _e,
enum mqtt_state {
  MQTT_STATE_LIST
  MQTT_STATE_MAX
};
#undef MQTT_STATE

static char const *mqtt_state_text( enum mqtt_state state ) {
  static char const * const state_text[MQTT_STATE_MAX] = {
    #define MQTT_STATE( _e,_t ) _t,
    MQTT_STATE_LIST
    #undef MQTT_STATE
  };

  char const *text = "Unknown";
  if( state<MQTT_STATE_MAX )
    text = state_text[state];

  return text;
}

struct mqtt_data {
  BaseType_t coreID;
  TaskHandle_t task;

  enum mqtt_state state;
  char root[20];
  char topic[32];

  esp_mqtt_client_config_t cfg;
  esp_mqtt_client_handle_t client;

  uint8_t info;
};

static struct mqtt_data *mqtt_ctxt( void ) {
  static struct mqtt_data mqtt = {
    .state = MQTT_IDLE,
	.root = CONFIG_MQTT_ROOT,
    .cfg = {
      .broker.address = {
        .uri = "mqtt://192.168.253.251:1883",
	  },
      .session = {
    	.last_will = {
          .msg = "offline",
          .topic = mqtt.topic,
          .qos = 1,
          .retain = 1,
    	},
		.keepalive=20,
      },
    },
	.info = 0,
  };

  static struct mqtt_data *ctxt = NULL;
  if( !ctxt ){
    ctxt = &mqtt;
    // Runtime Initialisation?
  }

  return ctxt;
}

static void log_error_if_nonzero( const char *message, int error_code ) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code );
  }
}

static bool mqtt_broker_configured( struct mqtt_data *ctxt ) {
  bool res = ( ctxt->cfg.broker.address.uri[0] != '\0' );

  return res;
}

static void mqtt_set_state( struct mqtt_data *ctxt, enum mqtt_state newState ) {
  if( ctxt ) {
    if( ctxt->state != newState ) {
      ESP_LOGI( TAG, "state %s->%s ", mqtt_state_text(ctxt->state), mqtt_state_text(newState) );
      ctxt->state = newState;
    }
  }
}

/****************************************************
 * Information topics
 */

typedef void (*info_field)( struct mqtt_data *ctxt, char *topic );

enum info_topic {
  INFO_FW,
  INFO_VER,
  // last
  INFO_MAX
};

static void publish_firmware( struct mqtt_data *ctxt, char *topic ) {
  const esp_app_desc_t *app = esp_app_get_description();
  strcat( topic, "/firmware" );
  esp_mqtt_client_publish( ctxt->client,topic, app->project_name, 0, 1, 1);
}

static void publish_version( struct mqtt_data *ctxt, char *topic ) {
  const esp_app_desc_t *app = esp_app_get_description();
  strcat( topic, "/version" );
  esp_mqtt_client_publish( ctxt->client,topic, app->version, 0, 1, 1);
}

static void mqtt_publish_info( struct mqtt_data *ctxt ) {
  static info_field const publish[INFO_MAX] = {
    [INFO_FW]  = publish_firmware,
	[INFO_VER] = publish_version,
  };
  if( ctxt->info < INFO_MAX ) {
    char topic[64];
    sprintf( topic, "%s/info", ctxt->topic );
    publish[ctxt->info]( ctxt, topic );
    ctxt->info++;
  }
}

/*******************************************************************************
 * RX message
 */
static void mqtt_publish_rx( struct mqtt_data *ctxt, char const *ts, char const *msg ) {
  char topic[64], rx[256];

  char *end = strstr( msg,"\r\n");
  if( end ) end[0] = '\0';

  sprintf( topic, "%s/rx", ctxt->topic );
  sprintf( rx , "{\"ts\":\"%s\", \"msg\":\"%s\"}",ts,msg);
  esp_mqtt_client_publish( ctxt->client,topic, rx, 0, 1, 1);
}

void MQTT_publish_rx( char const *ts, char const *msg ) {
  struct mqtt_data *ctxt= mqtt_ctxt();
  mqtt_publish_rx( ctxt, ts, msg );
}

/*******************************************************************************
 * TX message
 */
static void mqtt_subscribe_tx( struct mqtt_data *ctxt ) {
  char topic[64];
  sprintf( topic, "%s/tx", ctxt->topic );
  esp_mqtt_client_subscribe( ctxt->client, topic, 0);
}


static void mqtt_process_data( esp_mqtt_event_handle_t event ) {
  printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
  printf("DATA=%.*s\r\n", event->data_len, event->data);
}

		static void mqtt_event_handler( void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data )
{
  struct mqtt_data *ctxt = handler_args;

  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
  esp_mqtt_event_handle_t event = event_data;
//  esp_mqtt_client_handle_t client = event->client;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    mqtt_set_state( ctxt, MQTT_CONNECTED );
    break;

  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_DATA:
    mqtt_process_data(event);
	break;

  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if( event->error_handle->error_type==MQTT_ERROR_TYPE_TCP_TRANSPORT ) {
      log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;

  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

static void mqtt_state_machine( struct mqtt_data *ctxt ) {

  switch( ctxt->state ){
  case MQTT_IDLE:
    if( mqtt_broker_configured( ctxt ) ) {
    	mqtt_set_state( ctxt,MQTT_WAIT );
    }
    break;

  case MQTT_WAIT:
	if( wifi_is_connected() )
      mqtt_set_state( ctxt,MQTT_START );
	break;

  case MQTT_START:
	char const *dev = device();
	if( dev[0] ) {
      sprintf( ctxt->topic,"%s/%s",ctxt->root,dev );
      ctxt->client = esp_mqtt_client_init( &ctxt->cfg );
      ESP_LOGI( TAG, "Connecting to %s",ctxt->cfg.broker.address.uri );
      esp_mqtt_client_register_event( ctxt->client, ESP_EVENT_ANY_ID, mqtt_event_handler, ctxt );
      esp_mqtt_client_start( ctxt->client );
      mqtt_set_state( ctxt,MQTT_STARTING );
	}
    break;

  case MQTT_STARTING:
	break;

  case MQTT_CONNECTED:
    esp_mqtt_client_publish( ctxt->client, ctxt->topic, "online", 0, 1, 1);
    mqtt_subscribe_tx( ctxt );
	mqtt_set_state( ctxt,MQTT_ACTIVE );
    break;

  case MQTT_ACTIVE:
    if( ctxt->info<INFO_MAX )
      mqtt_publish_info( ctxt );
    break;

  default:
    break;
  };
}

static void Mqtt( void *param ) {
  struct mqtt_data *ctxt = param;

  ESP_LOGI( TAG, "Task Started");

  do {
	mqtt_state_machine( ctxt );
    vTaskDelay( 50/portTICK_PERIOD_MS );
  } while(1);
}

MQTT_HNDL ramses_mqtt_init( BaseType_t coreID ) {
  struct mqtt_data *ctxt= mqtt_ctxt();
  ctxt->coreID = coreID;

  esp_log_level_set(TAG, CONFIG_MQTT_LOG_LEVEL );

  xTaskCreatePinnedToCore( Mqtt, "MQTT", 4096, ctxt, 10, &ctxt->task, ctxt->coreID );

  return ctxt;
}
