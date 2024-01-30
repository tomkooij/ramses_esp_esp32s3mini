idf_component_register(
  SRCS "frame.c" "uart.c"
  INCLUDE_DIRS "."
  REQUIRES driver 
  PRIV_REQUIRES ramses-led ramses-debug cc1101 message
)
