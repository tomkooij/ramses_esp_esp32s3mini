idf_component_register(
    SRCS gateway.c device.c
    INCLUDE_DIRS "."
    REQUIRES console
    PRIV_REQUIRES message
)

