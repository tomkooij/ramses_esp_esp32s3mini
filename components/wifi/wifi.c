/********************************************************************
 * ramses_esp
 * wifi.c
 *
 * (C) 2023 Peter Price
 *
 * WiFi Station
 *
 */
#include <string.h>

static const char *TAG = "WIFI";
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "wifi.h"

/* Map configuration to internal constants */
#if CONFIG_WPA3_SAE_PWE_HUNT_AND_PECK
#define SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define H2E_IDENTIFIER ""
#elif CONFIG_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define H2E_IDENTIFIER CONFIG_WIFI_PW_ID
#elif CONFIG_WPA3_SAE_PWE_BOTH
#define SAE_MODE WPA3_SAE_PWE_BOTH
#define H2E_IDENTIFIER CONFIG_WIFI_PW_ID
#endif

#if CONFIG_WIFI_AUTH_OPEN
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_WIFI_AUTH_WEP
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_WIFI_AUTH_WPA_PSK
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_WIFI_AUTH_WPA2_PSK
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA_WPA2_PSK
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_WIFI_AUTH_WPA3_PSK
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WPA2_WPA3_PSK
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_WIFI_AUTH_WAPI_PSK
#define SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

struct wifi_data {
  BaseType_t coreID;
  TaskHandle_t   task;
  QueueHandle_t  queue;

  enum wifi_status status;
  int retry_num;

  /* FreeRTOS event group to signal when we are connected*/
  EventGroupHandle_t event_group;
  #define WIFI_CONNECTED_BIT BIT0
  #define WIFI_FAIL_BIT      BIT1

  esp_event_handler_instance_t any_id;
  esp_event_handler_instance_t got_ip;

  wifi_config_t station_config;
};

static struct wifi_data *wifi_ctxt( void ) {
  static struct wifi_data wifi = {
    .status = WIFI_IDLE,
    .station_config = {
      .sta = {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
        /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
         * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
         * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
         * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
         */
        .threshold.authmode = SCAN_AUTH_MODE_THRESHOLD,
        .sae_pwe_h2e = SAE_MODE,
        .sae_h2e_identifier = H2E_IDENTIFIER,
      },
    }
  };

  static struct wifi_data *ctxt = NULL;
  if( !ctxt ){
    ctxt = &wifi;
    // Runtime Initialisation?
  }

  return ctxt;
}

static void wifi_event_handler( void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  struct wifi_data *ctxt = arg;

  if( event_base==WIFI_EVENT && event_id==WIFI_EVENT_STA_START ) {
     esp_wifi_connect();
  } else if( event_base==WIFI_EVENT && event_id==WIFI_EVENT_STA_DISCONNECTED ) {
    if( ctxt->retry_num < CONFIG_WIFI_MAXIMUM_RETRY ) {
      esp_wifi_connect();
      ctxt->retry_num++;
      ESP_LOGI(TAG, "retry to connect to the AP");
    } else {
      xEventGroupSetBits( ctxt->event_group, WIFI_FAIL_BIT );
    }
    ESP_LOGI(TAG,"connect to the AP fail");
  } else if( event_base==IP_EVENT && event_id==IP_EVENT_STA_GOT_IP ) {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    ctxt->retry_num = 0;
    xEventGroupSetBits( ctxt->event_group, WIFI_CONNECTED_BIT );
  }
}

static void wifi_create( struct wifi_data *ctxt ) {
  ctxt->event_group = xEventGroupCreate();
  ESP_ERROR_CHECK( esp_netif_init() );

  ESP_ERROR_CHECK( esp_event_loop_create_default() );
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );

  ESP_ERROR_CHECK( esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,  &wifi_event_handler,ctxt, &ctxt->any_id) );
  ESP_ERROR_CHECK( esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP, &wifi_event_handler,ctxt, &ctxt->got_ip) );
}

static void wifi_start( struct wifi_data *ctxt ) {
  ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
  ESP_ERROR_CHECK( esp_wifi_set_config( WIFI_IF_STA, &ctxt->station_config ) );
  ESP_ERROR_CHECK( esp_wifi_start() );
}

static void wifi_status( struct wifi_data *ctxt ) {
  EventBits_t bits = 0;
  if( ctxt->status >= WIFI_CREATED )
    bits = xEventGroupGetBits( ctxt->event_group );

  switch( ctxt->status ){
  case WIFI_IDLE:
    if( ctxt->station_config.sta.ssid[0] != '\0' ) {
      wifi_create(ctxt);
      ctxt->status = WIFI_CREATED;
      ESP_LOGI( TAG, "WiFi created");
	}
    break;

  case WIFI_CREATED:
    wifi_start( ctxt );
    ctxt->status = WIFI_STARTED;
    ESP_LOGI( TAG, "WiFi started");
    break;

  case WIFI_STARTED:
    if( bits & WIFI_CONNECTED_BIT ) {
      ESP_LOGI( TAG, "connected to ap SSID:%s", (char *)ctxt->station_config.sta.ssid );
      ctxt->status = WIFI_CONNECTED;
    } else if( bits & WIFI_FAIL_BIT ) {
      ESP_LOGI( TAG, "Failed to connect to SSID:%s",(char *)ctxt->station_config.sta.ssid );
    }
    ctxt->status = WIFI_FAILED;
    break;

  case WIFI_FAILED:
    break;

  case WIFI_CONNECTED:
    break;

  case WIFI_DISCONNECTED:
    break;
  };
}

static void Wifi( void *param ) {
  struct wifi_data *ctxt = param;

  ESP_LOGI( TAG, "Task Started");

  do {
	wifi_status( ctxt );
    vTaskDelay( portTICK_PERIOD_MS/10 );
  } while(1);
}

WIFI_HNDL wifi_init( BaseType_t coreID ) {
  struct wifi_data *ctxt= wifi_ctxt();
  ctxt->coreID = coreID;

  esp_log_level_set(TAG, CONFIG_WIFI_LOG_LEVEL );

  xTaskCreatePinnedToCore( Wifi, "WiFi", 4096, ctxt, 10, &ctxt->task, ctxt->coreID );

  return ctxt;
}
