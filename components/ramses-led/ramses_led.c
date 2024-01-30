/********************************************************************
 * ramses_esp
 * ramses_led.c
 *
 * (C) 2023 Peter Price
 *
 * LED control
 *
 */
#include "ramses_led.h"

#include <stdio.h>

#include "driver/gpio.h"


#define GPIO_PIN_SEL(_p)  ( 1ULL<<(_p) )

static const struct led_map {
  uint32_t gpio;
} map[LED_MAX] = {
  { CONFIG_GPIO_RX_LED },   // RX
  { CONFIG_GPIO_TX_LED },   // TX
};

void ramses_led_init(void) {
  gpio_config_t io_conf = {
    .intr_type = GPIO_INTR_DISABLE,
    .mode = GPIO_MODE_OUTPUT,
    .pin_bit_mask = 0,
    .pull_down_en = 1,
    .pull_up_en = 0
  };

  enum LED_ID led;
  for( led=0 ;  led<LED_MAX ; led++ ) {
	io_conf.pin_bit_mask = GPIO_PIN_SEL( map[led].gpio );
    gpio_config(&io_conf);
    led_off( led );
  }
}

void led_on( enum LED_ID led ) {
  if( led<LED_MAX )
    gpio_set_level( map[led].gpio, 1 );
}

void led_off( enum LED_ID led ) {
 if( led<LED_MAX )
   gpio_set_level( map[led].gpio, 0 );
}
