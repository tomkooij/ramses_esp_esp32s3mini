/********************************************************************
 * ramses_esp
 * wifi.h
 *
 * (C) 2023 Peter Price
 *
 * WiFi Station
 *
 */
#ifndef _WIFI_H_
#define _WIFI_H_

enum wifi_status {
  WIFI_IDLE,
  WIFI_CREATED,
  WIFI_STARTED,
  WIFI_FAILED,
  WIFI_CONNECTED,
  WIFI_DISCONNECTED
};

typedef void * WIFI_HNDL;

extern WIFI_HNDL wifi_init( BaseType_t coreID );

#endif // _WIFI_H_
