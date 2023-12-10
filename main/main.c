/********************************************************************
 * ramses_esp
 * main.c
 *
 * (C) 2023 Peter Price
 *
 * ramses_esp is designed to run on a two core ESP32-S3
 *
 * The first core is used for the radio interface to the cc1101.
 * This core handles the RX and TX of RAMSES messages from the radio.
 *
 * The second core is responsible for communications with a host system.
 * It can implement a variety of protocols to transfer the message contents
 * between the ESP32 device and the host.
 */
#include "freertos/FreeRTOS.h"

#include "esp_debug.h"
#include "led.h"

#include "radio.h"
#include "host.h"

void app_main(void)
{
  esp_debug_init();
  led_init();

  Radio_init( CONFIG_RADIO_CORE );
  Host_init( CONFIG_HOST_CORE );
}
