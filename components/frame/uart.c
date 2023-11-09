/********************************************************************
 * ramses_esp
 * uart.c
 *
 * (C) 2023 Peter Price
 *
 * Radio UART interface between frame and cc1101
 * Provides an RX interface using UART data
 * Provides a TX interface using the cc1101 TX fifo
 *
 */
#include <stdio.h>
#include <string.h>

#include <driver/uart.h>

static const char * TAG = "UART";
#include "esp_log.h"
#include "esp_err.h"

#include "uart.h"
#include "frame.h"

static uart_port_t const uart_num = UART_NUM_1;
static QueueHandle_t uartQ;

/*******************************************************
* HW UART interface to cc1101
*/

/****************************************************************************/
enum uart_mode {
  uart_off,
  uart_rx,
  uart_tx
};

static enum uart_mode uartMode = uart_off;

/*******************************************************
* UART off
*/

static void uart_work_off(void) {
  uart_flush_input(uart_num);
}

/*******************************************************
* UART RX
*/

static void rx_start(void) {
  uart_flush_input(uart_num);
  uart_enable_rx_intr(uart_num);
}

//---------------------------------------------------------------------------------

static void rx_stop(void) {
  uart_disable_rx_intr(uart_num);
}

//---------------------------------------------------------------------------------

static void uart_work_rx(void) {
  uart_event_t event = {0};
  if( xQueueReceive( uartQ, &event, portTICK_PERIOD_MS  )) {
    if( event.size ) {
	    size_t i;
	    uint8_t dtmp[256];
	    uart_read_bytes( uart_num, dtmp, event.size, portTICK_PERIOD_MS/10 );
	    for( i=0 ; i<event.size ;  i++ )
		    frame_rx_byte( dtmp[i] );
	  }
  }
}

/*******************************************************
* UART TX
*/

static void tx_start(void) {
//  uart_enable_tx_intr(uart_num);
}

//---------------------------------------------------------------------------------

static void tx_stop(void) {
//  uart_disable_tx_intr(uart_num);
}

//---------------------------------------------------------------------------------

static void uart_work_tx(void) {
  uart_flush_input(uart_num);
}

/***************************************************************************
****************************************************************************
** External interface
****************************************************************************
****************************************************************************/

void uart_rx_enable(void) {
  tx_stop();
  rx_start();

  uartMode = uart_rx;
}

void uart_tx_enable(void) {
  rx_stop();
  tx_start();

  uartMode = uart_tx;
}

void uart_disable(void) {
  rx_stop();
  tx_stop();

  uartMode = uart_off;
}

void uart_work(void)
{
  switch( uartMode ) {
  case uart_off:  uart_work_off();  break;
  case uart_rx:   uart_work_rx();   break;
  case uart_tx:   uart_work_tx();   break;
  }
}

static uart_config_t const uart_config = {
  .baud_rate = RADIO_BAUDRATE,
  .data_bits = UART_DATA_8_BITS,
  .parity = UART_PARITY_DISABLE,
  .stop_bits = UART_STOP_BITS_1,
  .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
  .source_clk = UART_SCLK_DEFAULT
};

static uart_intr_config_t const uintr_cfg = {
  .intr_enable_mask = UART_DATA , //| UART_BREAK | UART_FIFO_OVF | UART_FRAME_ERR, // | UART_BUFFER_FULL ,          /*!< UART FIFO overflow event*/
  .rxfifo_full_thresh = 1,
};

void uart_init(void)
{
  esp_log_level_set(TAG, CONFIG_UART_LOG_LEVEL );

  ESP_ERROR_CHECK( uart_set_pin( uart_num, CONFIG_CC_GDO0_GPIO, CONFIG_CC_GDO2_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) );
  ESP_ERROR_CHECK( uart_param_config( uart_num, &uart_config ) );

  //Install UART driver, and get the queue.
  ESP_ERROR_CHECK( uart_driver_install( uart_num, 256,0 , 16,&uartQ, 0 ) );

  ESP_ERROR_CHECK( uart_intr_config( uart_num, &uintr_cfg ) );

  ESP_LOGI( TAG, "initialised" );
}
