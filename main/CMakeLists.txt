# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c radio.c host.c
    INCLUDE_DIRS
    PRIV_INCLUDE_DIRS
    REQUIRES 
    PRIV_REQUIRES console driver nvs_flash led wifi cc1101 frame message gateway esp_debug
)
