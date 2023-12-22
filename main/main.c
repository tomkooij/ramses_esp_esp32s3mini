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
#include <stdio.h>
#include "esp_system.h"
#include "esp_app_desc.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"

#include "esp_debug.h"
#include "led.h"
#include "wifi.h"

#include "radio.h"
#include "host.h"

void app_main(void)
{
  const esp_app_desc_t *app = esp_app_get_description();
  printf("# %s %s\n",app->project_name, app->version );

  esp_debug_init();
  led_init();

  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if( ret==ESP_ERR_NVS_NO_FREE_PAGES || ret==ESP_ERR_NVS_NEW_VERSION_FOUND ) {
    ESP_ERROR_CHECK( nvs_flash_erase() );
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  wifi_init( CONFIG_HOST_CORE );

  Radio_init( CONFIG_RADIO_CORE );
  Host_init( CONFIG_HOST_CORE );

  gpio_reset_pin( CONFIG_FUNCTION_PIN );
  gpio_set_direction( CONFIG_FUNCTION_PIN, GPIO_MODE_INPUT );

  /* Read the status of GPIO0.
   * If GPIO0 is LOW for longer than 1000 ms then restart it
   */
  while( 1 ) {
    if( gpio_get_level(CONFIG_FUNCTION_PIN)==0 ) {
      vTaskDelay( 1000 / portTICK_PERIOD_MS );
      if( gpio_get_level(CONFIG_FUNCTION_PIN)==0 ) {
        printf("Restarting...\n");
        fflush(stdout);
        // Wait for release
        while( ( gpio_get_level(CONFIG_FUNCTION_PIN)==0 ) )
          vTaskDelay( 200 / portTICK_PERIOD_MS );
        esp_restart();
      }
    }
    vTaskDelay( 200 / portTICK_PERIOD_MS );
  }
}
