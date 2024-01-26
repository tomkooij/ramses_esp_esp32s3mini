/********************************************************************
 * ramses_esp
 * network.h
 *
 * (C) 2023 Peter Price
 *
 * Network
 *
 */
#ifndef _NETWORK_H_
#define _NETWORK_H_

extern void NET_set_mqtt_broker( char *new_server );
extern void NET_set_sntp_server( char *new_server );

extern void network_init( BaseType_t coreID );

#endif // _NETWORK_H_
