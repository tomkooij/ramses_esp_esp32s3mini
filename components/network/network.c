/********************************************************************
 * ramses_esp
 * network.c
 *
 * (C) 2023 Peter Price
 *
 * Network
 *
 */

#include "wifi.h"
#include "ota.h"

#include "network.h"

void network_init( BaseType_t coreID ) {
  ota_init( coreID );
  wifi_init( coreID );
}
