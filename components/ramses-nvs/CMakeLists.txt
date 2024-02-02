idf_component_register(
  SRCS "ramses_nvs.c"
  INCLUDE_DIRS "."
  REQUIRES console nvs_flash
  PRIV_REQUIRES command
)
