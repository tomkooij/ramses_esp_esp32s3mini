/********************************************************
 * ramses_esp
 * device.h
 *
 * Device ID
 *
 *******************************************************/
#include <stddef.h>
#include <esp_mac.h>

#include "device.h"

static uint8_t  DevClass;
static uint32_t DevId;
static char     Dev[12];

void device_init( uint8_t class ) {
  uint8_t mac[6];
  esp_base_mac_addr_get( mac );

  DevClass = class;
  DevId = (  ( (uint32_t)mac[3] << 16 )
           + ( (uint32_t)mac[4] <<  8 )
           + ( (uint32_t)mac[5] <<  0 )
          ) & 0x3FFFF;
}

void device_get_id( uint8_t *class, uint32_t *id ) {
  if( class ) *class = DevClass;
  if( id    ) *id    = DevId;
}

char const *device(void) {
  if( Dev[0]=='\0' && DevClass!=0 )
    sprintf( Dev, "%02d:%06ld",DevClass,DevId );

  return Dev;
}
