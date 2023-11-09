idf_component_register(
  SRCS "frame.c" "uart.c"
  PRIV_REQUIRES driver cc1101 message
  INCLUDE_DIRS "."
)
