idf_component_register(
  SRCS network.c wifi.c wifi_cmd.c ota.c ota_cmd.c
  INCLUDE_DIRS "."
  PRIV_REQUIRES esp_wifi console mbedtls app_update esp_partition esp_https_ota esp_http_client
)

