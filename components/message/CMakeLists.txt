idf_component_register(
    SRCS message.c
    INCLUDE_DIRS .
    REQUIRES gateway
)
component_compile_options(-Wimplicit-fallthrough)
