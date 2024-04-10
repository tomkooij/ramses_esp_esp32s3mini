idf_component_register(
  SRCS "ramses_debug.c" "debug_cmd.c"
  INCLUDE_DIRS "."
  REQUIRES driver
  PRIV_REQUIRES command
  
)

