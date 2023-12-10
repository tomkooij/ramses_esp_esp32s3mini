/*
 * ramses_esp
 * led.h
 *
 *  Created on: 29 Nov 2023
 *      Author: peter
 */

#ifndef COMPONENTS_LED_LED_H_
#define COMPONENTS_LED_LED_H_

enum LED_ID {
  LED_RX,
  LED_TX,
  LED_MAX
};

void led_init(void);
void led_on( enum LED_ID led );
void led_off( enum LED_ID led);

#endif /* COMPONENTS_LED_LED_H_ */
