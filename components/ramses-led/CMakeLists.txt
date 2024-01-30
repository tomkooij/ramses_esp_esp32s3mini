set(component_srcs "ramses_led.c")

idf_component_register(
  SRCS "${component_srcs}"
  PRIV_REQUIRES driver
  INCLUDE_DIRS "."
)
