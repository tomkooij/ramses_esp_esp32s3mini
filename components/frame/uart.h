/********************************************************************
 * ramses_esp
 * uart.h
 *
 * (C) 2023 Peter Price
 *
 * Radio UART public API
 *
 */
#ifndef _UART_H_
#define _UART_H_

extern void uart_rx_enable(void);
extern void uart_tx_enable(void);
extern void uart_disable(void);

extern void uart_init(void);
extern void uart_work(void);

#define RADIO_BAUDRATE 38400

#endif // _UART_H_
