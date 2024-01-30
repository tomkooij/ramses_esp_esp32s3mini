idf_component_register(
  SRCS "frame.c" "uart.c"
  PRIV_REQUIRES driver ramses-led cc1101 message esp_debug
  INCLUDE_DIRS "."
)
