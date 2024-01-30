/********************************************************************
 * ramses_esp
 * ramses_network.h
 *
 * (C) 2023 Peter Price
 *
 * Network
 *
 */
#ifndef _RAMSES_NETWORK_H_
#define _RAMSES_NETWORK_H_

extern void NET_set_mqtt_broker( char *new_server );
extern void NET_set_sntp_server( char *new_server );

extern void ramses_network_init( BaseType_t coreID );

#endif // _RAMSES_NETWORK_H_
