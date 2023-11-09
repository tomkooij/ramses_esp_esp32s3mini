/********************************************************************
 * ramses_esp
 * gateway.h
 *
 * (C) 2023 Peter Price
 *
 * Gateway public API
 *
 */
#ifndef _GATEWAY_H_
#define _GATEWAY_H_

#include "freertos/FreeRTOS.h"

#include "message.h"
void gateway_radio_rx( struct message **message );

void gateway_init( BaseType_t coreID );

#endif // _GATEWAY_H_
