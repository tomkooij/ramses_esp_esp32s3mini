#include <string.h>
#include <util/delay.h>

#include <avr/interrupt.h>

#include "config.h"
#include "message.h"
#include "frame.h"

#define DEBUG_FRAME(_v)    DEBUG3(_v)

/***********************************************************************************
** RX Frame state machine
*/

enum frame_states {
  FRM_OFF,
  FRM_IDLE,
  FRM_RX,
  FRM_TX,
};

static struct frame_state {
  uint8_t state;
} frame;

static void frame_reset(void) {
  memset( &frame, 0, sizeof(frame) );
}

/*******************************************************
* Manchester Encoding
*
* The [Evo Message] is encoded in the bitstream with a
* Manchester encoding.  This is the only part of the
* complete packet encoded in this way so we cannot
* use the built in function of the CC1101
*
* While the bitstream is interpreted as a Big-endian stream
* the manchester codes inserted in the stream are little-endian
*
* The Manchester data here is designed to correspond with the
* Big-endian byte stream seen in the bitstream.
*
********
* NOTE *
********
* The manchester decode process converts the data from
*     8 bit little-endian to 4 bit big-endian
* The manchester encode process converts the data from
*     4 bit big-endian to 8 bit little-endian
*
* Since only a small subset of 8-bit values are actually allowed in
* the bitstream rogue values can be used to identify some errors in
* the bitstream.
*
*/

// Convert big-endian 4 bits to little-endian byte
static uint8_t const man_encode[16] = {
  0xAA, 0xA9, 0xA6, 0xA5,  0x9A, 0x99, 0x96, 0x95,
  0x6A, 0x69, 0x66, 0x65,  0x5A, 0x59, 0x56, 0x55
};

// Convert little-endian 4 bits to 2-bit big endian
static uint8_t man_decode[16] = {
  0xF, 0xF, 0xF, 0xF, 0xF, 0x3, 0x2, 0xF,
  0xF, 0x1, 0x0, 0xF, 0xF, 0xF, 0xF, 0xF
};

static inline int manchester_code_valid( uint8_t code ) {
 return ( man_decode[(code>>4)&0xF]!=0xF ) && ( man_decode[(code   )&0xF]!=0xF ) ;
}

static inline uint8_t manchester_decode( uint8_t byte ) {
  uint8_t decoded;

  decoded  = man_decode[( byte    ) & 0xF ];
  decoded |= man_decode[( byte>>4 ) & 0xF ]<<2;

  return decoded;
}

static inline uint8_t manchester_encode( uint8_t value ) {
  return man_encode[ value & 0xF ];
}

/***********************************************************************************
** RX FRAME processing
**
** A frame consists of <header><messaeg><trailer>
**  <header>  is a fixed sequence of bytes that identify the packet as an evohome packet
**  <message> varaiable length evohome message.. Data is manchester encoded.
** <trailer>  is a single byte (not a valid manchester code value) that marks end of packet
*/

enum frame_rx_states {
  FRM_RX_OFF,
  FRM_RX_IDLE,
  FRM_RX_SYNCH,
  FRM_RX_MESSAGE,
  FRM_RX_DONE,
  FRM_RX_ABORT
};

static struct frame_rx {
  uint8_t  state;

  uint8_t nBytes;
  uint8_t nRaw;
  uint8_t *raw;

  uint32_t syncBuffer;

  uint8_t count;
  uint8_t msgErr;
  uint8_t msgByte;
} rxFrm;

static void frame_rx_reset(void) {
  memset( &rxFrm, 0, sizeof(rxFrm) );
}

static uint8_t evo_hdr[] = { 0x33, 0x55, 0x53 };
static uint8_t evo_tlr[] = { 0x35 };
static uint32_t const syncWord = 0x00335553;

void frame_rx_byte(uint8_t byte) {
  switch( rxFrm.state ) {
  case FRM_RX_IDLE:
    rxFrm.state = FRM_RX_SYNCH;
    // Fall through

  case FRM_RX_SYNCH:
    rxFrm.syncBuffer <<= 8;
    rxFrm.syncBuffer |= byte;
    rxFrm.syncBuffer &= 0x00FFFFFF;

    if( rxFrm.syncBuffer == syncWord )
    {
      rxFrm.raw = msg_rx_start();
      if( rxFrm.raw ) {
        rxFrm.nRaw = rxFrm.raw[0];
        rxFrm.state  = FRM_RX_MESSAGE;
        DEBUG_FRAME(1);
      }
    }
    break;

  case FRM_RX_MESSAGE:
    if( byte==0x00 ) {
      rxFrm.state = FRM_RX_ABORT;
      rxFrm.msgErr = MSG_CLSN_ERR;
    } else if( byte==FRM_LOST_SYNC ) {
      rxFrm.state = FRM_RX_ABORT;
      rxFrm.msgErr = MSG_SYNC_ERR;
    } else if( byte == evo_tlr[0] ) {
      rxFrm.state = FRM_RX_DONE;
    } else {
      rxFrm.raw[rxFrm.nBytes++] = byte;

      if( !manchester_code_valid( byte ) ) {
        rxFrm.state = FRM_RX_ABORT;
        rxFrm.msgErr = MSG_MANC_ERR;
      } else {
        rxFrm.msgByte <<= 4;
        rxFrm.msgByte |= manchester_decode( byte );
        rxFrm.count = 1- rxFrm.count;

        if( !rxFrm.count ) {
          rxFrm.msgErr = msg_rx_byte( rxFrm.msgByte );
          if( rxFrm.msgErr != MSG_OK )
            rxFrm.state = FRM_RX_ABORT;
        }
      }
    }
    break;
  }

  // Protect raw data buffer
  if( rxFrm.state > FRM_RX_SYNCH && rxFrm.state < FRM_RX_DONE ) {
    if( rxFrm.nBytes >= rxFrm.nRaw ) {
      rxFrm.state = FRM_RX_ABORT;
      rxFrm.msgErr = MSG_OVERRUN_ERR;
    }
  }

  if( rxFrm.state >= FRM_RX_DONE ) {
    DEBUG_FRAME(0);
  }

}

static void frame_rx_done(void) {
  DEBUG_FRAME(1);

  // Reset rxFrm as quickly as possible after collision can pick up new frame header
  uint8_t nBytes = rxFrm.nBytes;
  uint8_t msgErr = rxFrm.msgErr;
  uint8_t rssi;

  frame_rx_reset();

  // Now tell message about the end of frame
  rssi = cc_read_rssi();
  msg_rx_rssi( rssi );
  msg_rx_end(nBytes,msgErr);

  DEBUG_FRAME(0);
}

/***********************************************************************************
** TX FRAME processing
**
** We must provide the following to the uart
**   <prefix><message><suffix>
**
**   <prefix>  = <preamble><sync word><header>
**   <message> = < manchester encoded pairs of bytes >
**   <suffix>  = <trailer><training>
**
*/

enum frame_tx_states {
  FRM_TX_OFF,
  FRM_TX_READY,
  FRM_TX_IDLE,
  FRM_TX_PREFIX,
  FRM_TX_MESSAGE,
  FRM_TX_SUFFIX,
  FRM_TX_DONE
};

static struct frame_tx {
  uint8_t state;

  uint8_t nBytes;
  uint8_t nRaw;
  uint8_t *raw;

  uint8_t count;
  uint8_t msgByte;
} txFrm;

static void frame_tx_reset(void) {
  memset( &txFrm, 0, sizeof(txFrm) );
}

static uint8_t tx_prefix[] = {
  0x55, 0x55, 0x55, 0x55, 0x55,   // Pre-amble
  0xFF, 0x00,                     // Sync Word
  0x33, 0x55, 0x53                // Header
};

static uint8_t tx_suffix[] = {
  0x35,                           // Trailer
  0x55, 0x55, 0x55,               // Training
};

void frame_tx_start( uint8_t *raw, uint8_t nRaw ) {
  txFrm.raw = raw;
  txFrm.nRaw = nRaw;

  txFrm.state = FRM_TX_READY;
}

uint8_t frame_tx_byte(void) {
  uint8_t byte = 0x00;
  uint8_t done = 0;

  switch( txFrm.state ) {
  case FRM_TX_IDLE:
    txFrm.state = FRM_TX_PREFIX;
    // Fall through

  case FRM_TX_PREFIX:
    if( txFrm.count < sizeof(tx_prefix) ) {
      byte = tx_prefix[txFrm.count++];
      break;
    }
    txFrm.count = 0;
    txFrm.state = FRM_TX_MESSAGE;
    // Fall through

  case FRM_TX_MESSAGE:
    if( ( txFrm.count&1 ) == 0 )
      txFrm.msgByte = msg_tx_byte(&done);

    if( !done ) {
      byte = manchester_encode( txFrm.msgByte >> 4 );
      txFrm.msgByte <<= 4;
      txFrm.count++;

      if( txFrm.raw && txFrm.nBytes<txFrm.nRaw )
        txFrm.raw[ txFrm.nBytes++ ] = byte;

      break;
    }
    msg_tx_end( txFrm.nBytes );

    txFrm.count = 0;
    txFrm.state = FRM_TX_SUFFIX;
    // Fall through

  case FRM_TX_SUFFIX:
    if( txFrm.count < sizeof(tx_suffix) ) {
      byte = tx_suffix[txFrm.count++];
      break;
    }
    txFrm.count = 0;
    txFrm.state = FRM_TX_DONE;
    // Fall through

  case FRM_TX_DONE:
    break;
  }

  return byte;
}

static void frame_tx_done(void) {
  msg_tx_done();
  frame_tx_reset();
}

/***************************************************************************
** External interface
*/

static void frame_rx_enable(void) {
  cc_enter_rx_mode();
  uart_rx_enable();

  frame.state = FRM_RX;
  rxFrm.state = FRM_RX_IDLE;
}

static void frame_tx_enable(void) {
//  cc_enter_tx_mode();
//  uart_tx_enable();

  frame.state = FRM_TX;
  txFrm.state = FRM_TX_IDLE;
}

void frame_disable(void) {
  uart_disable();
  cc_enter_idle_mode();

  frame.state = FRM_OFF;
}

void frame_init(void) {
  frame_reset();
  uart_init();

  frame.state = FRM_IDLE;
}


void frame_work(void) {
  switch( frame.state ) {
  case FRM_IDLE:
    if( rxFrm.state==FRM_RX_OFF ) {
      frame_rx_enable();
    }
    break;

  case FRM_RX:
    if( rxFrm.state>=FRM_RX_DONE ) {
      frame_rx_done();
    }
    if( rxFrm.state<=FRM_RX_IDLE ) {
      if( txFrm.state==FRM_TX_READY ) {
        frame_tx_enable();
      } else if( rxFrm.state==FRM_RX_OFF ) {
        frame_rx_enable();
      }
    }
    break;

  case FRM_TX:
    if( txFrm.state>=FRM_TX_DONE ) {
      frame_tx_done();
      frame_rx_enable();
    }
    // TEST ONLY
    else if( txFrm.state>FRM_TX_READY ) frame_tx_byte();

    break;
  }

}
