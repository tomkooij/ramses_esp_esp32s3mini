/********************************************************************
 * ramses_esp
 * msg_10A0.c
 *
 * (C) 2025 Peter Price
 *
 * RAMSES message handling
 *
 * Process OPCODE=10A0 (DHW Setpoint)
 *
 */

#include "message.h"
#include "msg.h"

void msg_10A0_request_tx( uint32_t devAddr, uint32_t ctlAddr, uint8_t len, uint8_t *payload ) {
  uint32_t addr[MAX_ADDR];
  struct message *msg;

  addr[0] = devAddr ;
  addr[1] = ctlAddr ;
  addr[2] = 0 ;

  msg = msg_create( F_RQ, addr, 0x10A0, len, payload );
  if( msg!=NULL )
	msg_tx_ready( &msg );
}

void msg_10A0_reply_rx( struct message *msg, uint8_t *len, uint8_t **payload ) {
  if( msg ) {
	uint8_t l, *p;
    msg_get_payload( msg, &l, &p );

    if( len ) (*len) = l;
    if( payload && *payload ) (*payload) = p;
  }
}
