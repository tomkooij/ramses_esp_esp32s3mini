/********************************************************************
 * ramses_esp
 * network.c
 *
 * (C) 2023 Peter Price
 *
 * Network
 *
 */
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "esp_netif.h"
#include "nvs.h"

#include "ramses_sntp.h"
#include "wifi.h"
#include "ota.h"

// NVS identifiers
#define NETWORK_NAMESPACE "network"
#define MQTT_BROKER "mqtt_broker"
#define SNTP_SERVER "sntp_server"

#include "network_cmd.h"
#include "network.h"

/********************************************************************************
 * Network context
 */

struct network_data {
  nvs_handle_t nvs;

  size_t mqtt_broker_len;
  char *mqtt_broker;
  size_t sntp_server_len;
  char *sntp_server;
};

static struct network_data *network_ctxt( void ) {
  static struct network_data data ={
  };
  static struct network_data * ctxt = NULL;
  if( !ctxt ) {
	ctxt = &data;
	// Runtime initialisation ?
  }

  return ctxt;
}

/********************************************************************************
 * MQTT broker uri
 */

static void net_get_mqtt_broker( struct network_data *ctxt ) {
  esp_err_t err  = nvs_get_str( ctxt->nvs, MQTT_BROKER, NULL, &ctxt->mqtt_broker_len );
  if( err==ESP_OK ) {
    if( ctxt->mqtt_broker_len!=0 ) {
      ctxt->mqtt_broker = malloc( ctxt->mqtt_broker_len );	// length includes '\0' terminator
      if( ctxt->mqtt_broker ) {
        nvs_get_str( ctxt->nvs, MQTT_BROKER, ctxt->mqtt_broker, &ctxt->mqtt_broker_len );
      }
    }
  }
}

static void net_set_mqtt_broker( struct network_data *ctxt, char *new_broker ) {
  if( !ctxt->mqtt_broker || strcmp( ctxt->mqtt_broker, new_broker ) ) {
    size_t len = strlen( new_broker );
    if( len >= ctxt->mqtt_broker_len ) {
      if( ctxt->mqtt_broker ) free( ctxt->mqtt_broker );
      ctxt->mqtt_broker_len = len+1; // Need space for '\0' terminator
      ctxt->mqtt_broker = malloc( ctxt->mqtt_broker_len );
    }

    if( len>0 && ctxt->mqtt_broker ) {
        strcpy( ctxt->mqtt_broker, new_broker );
        nvs_set_str( ctxt->nvs, MQTT_BROKER, ctxt->mqtt_broker );
    }
  }
}

void NET_set_mqtt_broker( char *new_broker ) {
  struct network_data *ctxt = network_ctxt();
  if( ctxt && new_broker )
    net_set_mqtt_broker( ctxt, new_broker );
}


/********************************************************************************
 * SNTP server uri
 */

static void net_get_sntp_server( struct network_data *ctxt ) {
  esp_err_t err  = nvs_get_str( ctxt->nvs, SNTP_SERVER, NULL, &ctxt->sntp_server_len );
  if( err==ESP_OK ) {
    if( ctxt->sntp_server_len!=0 ) {
      ctxt->sntp_server = malloc( ctxt->sntp_server_len );	// length includes '\0' terminator
      if( ctxt->sntp_server ) {
        nvs_get_str( ctxt->nvs, SNTP_SERVER, ctxt->sntp_server, &ctxt->sntp_server_len );
      }
    }
  }
}

static void net_set_sntp_server( struct network_data *ctxt, char *new_server ) {
  if( !ctxt->sntp_server || strcmp( ctxt->sntp_server, new_server ) ) {
    size_t len = strlen( new_server );
    if( len >= ctxt->sntp_server_len ) {
      if( ctxt->sntp_server ) free( ctxt->sntp_server );
      ctxt->sntp_server_len = len+1; // Need space for '\0' terminator
      ctxt->sntp_server = malloc( ctxt->sntp_server_len );
    }

    if( len > 0 ) {
      if( ctxt->sntp_server ) {
        strcpy( ctxt->sntp_server, new_server );
        nvs_set_str( ctxt->nvs, SNTP_SERVER, ctxt->sntp_server );
      }
    }
  }
}

void NET_set_sntp_server( char *new_server ) {
  struct network_data *ctxt = network_ctxt();
  if( ctxt && new_server )
    net_set_sntp_server( ctxt, new_server );
}


/********************************************************************************
 * Initialisation
 */
void network_init( BaseType_t coreID ) {
  struct network_data *ctxt = network_ctxt();

  network_register();

  ESP_ERROR_CHECK( esp_netif_init() );

  esp_err_t err = nvs_open( NETWORK_NAMESPACE, NVS_READWRITE, &ctxt->nvs );
  if( err==ESP_OK ) {
    net_get_mqtt_broker( ctxt );
    net_get_sntp_server( ctxt );
  }

  ramses_sntp_init( coreID, ctxt->sntp_server );
  ota_init( coreID );
  wifi_init( coreID );
}
