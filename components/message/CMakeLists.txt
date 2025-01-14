set(component_srcs "message.c" "msg.c"  "msg_0016.c" "msg_10A0.c" "msg_1260.c" "msg_1FC9.c" )

idf_component_register(
    SRCS "${component_srcs}"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES gateway frame
)

component_compile_options(-Wimplicit-fallthrough)
