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

typedef void * WIFI_HNDL;

extern void wifi_set_ssid( char *ssid );
extern void wifi_set_password( char *password );
extern void wifi_restart( void );
extern void wifi_status( void );

extern WIFI_HNDL wifi_init( BaseType_t coreID );

#endif // _WIFI_H_
