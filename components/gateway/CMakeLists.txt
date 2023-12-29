idf_component_register(
    SRCS gateway.c device.c
    INCLUDE_DIRS "."
    PRIV_REQUIRES message console
)

