/********************************************************************
 * ramses_esp
 * msg_1FC9.c
 *
 * (C) 2025 Peter Price
 *
 * RAMSES message handling
 *
 * Process OPCODE=1FC9 (Bind sequence)
 *
 */

#include "message.h"
#include "msg.h"

void msg_1FC9_request_tx( uint32_t devAddr, uint8_t len, uint8_t *payload ) {
  uint32_t addr[MAX_ADDR];
  struct message *msg;

  addr[0] = devAddr ;
  addr[1] = 0 ;
  addr[2] = devAddr ;

  msg = msg_create( F_I, addr, 0x1FC9, len, payload );
  if( msg!=NULL )
	msg_tx_ready( &msg );
}

void msg_1FC9_reply_rx( struct message *msg ) {
  uint8_t len;
  uint8_t *payload;

  if( msg ) {
    msg_get_payload( msg, &len, &payload );
  }
}

void msg_1FC9_ack_tx( uint32_t devAddr, uint32_t ctlAddr, uint8_t len, uint8_t *payload ) {
  uint32_t addr[MAX_ADDR];
  struct message *msg;

  addr[0] = devAddr ;
  addr[1] = ctlAddr ;
  addr[2] = 0 ;

  msg = msg_create( F_I, addr, 0x1FC9, len, payload );
  if( msg!=NULL )
	msg_tx_ready( &msg );
}
