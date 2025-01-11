/********************************************************************
 * ramses_esp
 * msg.c
 *
 * (C) 2023 Peter Price
 *
 * RAMSES message handling
 * General message utilities
 *
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "message.h"
#include "msg.h"

void msg_reset( struct message *msg ) {
  if( msg != NULL ) {
    memset( msg, 0, sizeof(*msg) );
  }
}

char *msg_timestamp( char *timestamp, int len ) {
  struct timeval tv;
  struct tm *nowtm;
  ssize_t written = -1;

  gettimeofday(&tv, NULL);
  nowtm = localtime(&tv.tv_sec);

  written = (ssize_t)strftime( timestamp,len, "%Y-%m-%dT%H:%M:%S", nowtm);
  if (( written>0 ) && ( (size_t)(written+7)<len ))
    written += snprintf( timestamp+written, len-(size_t)written, ".%06ld", tv.tv_usec);
  if (( written>0 ) && ( (size_t)(written+6)<len )) {
	char tz[8];
	strftime( tz,8, "%z", nowtm );
	// strftime does not generate ISO 8601 timezone - there's no ':'
	written += sprintf( timestamp+written,"%c%c%c:%c%c", tz[0],tz[1],tz[2],tz[3],tz[4] );
  }

  return timestamp;
}

char const *msg_get_ts( struct message const  *msg ){ return msg->timestamp; }

/********************************************************
** Message Header
********************************************************/

static const uint8_t address_flags[4] = {
  F_ADDR0+F_ADDR1+F_ADDR2 ,
                  F_ADDR2 ,
  F_ADDR0+        F_ADDR2 ,
  F_ADDR0+F_ADDR1
};

#define HDR_T_MASK 0x30
#define HDR_T_SHIFT   4
#define HDR_A_MASK 0x0C
#define HDR_A_SHIFT   2
#define HDR_PARAM0 0x02
#define HDR_PARAM1 0x01

uint8_t msg_get_hdr_flags(uint8_t header ) {
  uint8_t flags;

  flags = ( header & HDR_T_MASK ) >> HDR_T_SHIFT;   // Message type
  flags |= address_flags[ ( header & HDR_A_MASK ) >> HDR_A_SHIFT ];
  if( header & HDR_PARAM0 ) flags |= F_PARAM0;
  if( header & HDR_PARAM1 ) flags |= F_PARAM1;

  return flags;
}

uint8_t msg_get_header( uint8_t flags ) {
  uint8_t i;

  uint8_t header = 0xFF;
  uint8_t addresses = flags & ( F_ADDR0+F_ADDR1+F_ADDR2 );

  for( i=0 ; i<sizeof(address_flags) ; i++ ) {
    if( addresses==address_flags[i] ) {
      header = i << HDR_A_SHIFT;
      header |= ( flags & F_MASK ) << HDR_T_SHIFT;  // Message type
      if( flags & F_PARAM0 ) header |= HDR_PARAM0;
      if( flags & F_PARAM1 ) header |= HDR_PARAM1;
      break;
    }
  }

  return header;
}

/********************************************************
** Message Checksum
********************************************************/
uint8_t msg_checksum( struct message *msg ) {
  uint8_t csum;
  uint8_t i,j;

  // Missing fields will be zero so we can just add them to checksum without testing presence
                                                  { csum  = msg_get_header(msg->fields); }
  for( i=0 ; i<3 ; i++ ) { for( j=0 ; j<3 ; j++ ) { csum += msg->addr[i][j]; }       }
  for( i=0 ; i<2 ; i++ )                          { csum += msg->param[i];           }
  for( i=0 ; i<2 ; i++ )                          { csum += msg->opcode[i];          }
                                                  { csum += msg->len;                }
  for( i=0 ; i<msg->nPayload ; i++ )              { csum += msg->payload[i];         }

  return -csum;
}

/********************************************************
** Address conversion
********************************************************/
void msg_set_address( uint8_t *addr, uint8_t class, uint32_t id ) {
  addr[0] = ( ( class<< 2 ) & 0xFC ) | ( ( id >> 16 ) & 0x03 );
  addr[1] =                            ( ( id >>  8 ) & 0xFF );
  addr[2] =                            ( ( id       ) & 0xFF );
}

void msg_get_address( uint8_t *addr, uint8_t *class, uint32_t *id ) {
  uint8_t Class =          ( addr[0] & 0xFC ) >>  2;
  uint32_t Id =  (uint32_t)( addr[0] & 0x03 ) << 16
               | (uint32_t)( addr[1]        ) <<  8
               | (uint32_t)( addr[2]        )       ;

  if( class ) (*class) = Class;
  if( id    ) (*id   ) = Id;
}

/************************************************************************************
**
** Message status functions
**/
uint8_t msg_isValid( struct message *msg ) {
  uint8_t isValid = 0;
  
  if( msg ) {
	isValid = ( msg->error==MSG_OK );
  }
  
  return isValid;
}

uint8_t msg_isTx( struct message *msg ) {
  uint8_t isTx = 0;
  
  if( msg ) {
	isTx = msg_isValid( msg ) && ( msg->rssi==0 );
  }

  return isTx;
}

