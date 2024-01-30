idf_component_register(
  SRCS "ramses-mqtt.c"
  INCLUDE_DIRS "."
  REQUIRES mqtt 
  PRIV_REQUIRES ramses-network gateway
)

