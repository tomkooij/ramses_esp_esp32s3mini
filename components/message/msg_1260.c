/********************************************************************
 * ramses_esp
 * msg_1260.c
 *
 * (C) 2025 Peter Price
 *
 * RAMSES message handling
 *
 * Process OPCODE=1260 (DHW temp)
 *
 */

#include "message.h"
#include "msg.h"

void msg_1260_tx( uint32_t devAddr, uint8_t zone, uint16_t temp ) {
  uint32_t addr[MAX_ADDR];
  uint8_t payload[3];
  struct message *msg;

  addr[0] = devAddr ;
  addr[1] = 0 ;
  addr[2] = devAddr ;

  payload[0] = zone ;
  payload[1] = ( temp>>8 ) & 0xff ;
  payload[2] = ( temp>>0 ) & 0xff ;

  msg = msg_create( F_I, addr, 0x1260, sizeof(payload), payload );
  if( msg!=NULL )
	msg_tx_ready( &msg );
}
