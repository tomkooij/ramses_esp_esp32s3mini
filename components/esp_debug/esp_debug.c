/********************************************************************
 * ramses_esp
 * esp_debug.c
 *
 * (C) 2023 Peter Price
 *
 * Simple GPIO debug
 */

#include "../esp_debug/esp_debug.h"

void esp_debug_init(void) {
  if( DEBUG_MASK ) {
    gpio_config_t io_conf = {
      .intr_type = GPIO_INTR_DISABLE,     //disable interrupt
      .mode = GPIO_MODE_OUTPUT,           //set as output mode
      .pin_bit_mask = DEBUG_MASK,
      .pull_down_en = 1,                  //disable pull-down mode
	  .pull_up_en = 0 };                  //disable pull-up mode

    gpio_config(&io_conf);

    DEBUG1_OFF; DEBUG2_OFF; DEBUG3_OFF; DEBUG4_OFF; DEBUG5_OFF; DEBUG6_OFF;
  };
}
