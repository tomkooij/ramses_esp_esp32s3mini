idf_component_register(
  SRCS network.c network_cmd.c wifi.c wifi_cmd.c ramses_ota.c ota_cmd.c ramses_sntp.c
  INCLUDE_DIRS "."
  REQUIRES nvs_flash esp_wifi esp_netif console mbedtls app_update esp_partition esp_https_ota esp_http_client
  PRIV_REQUIRES 
)

