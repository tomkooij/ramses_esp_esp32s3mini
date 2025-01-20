# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS main.c radio.c host.c platform.c
    INCLUDE_DIRS
    PRIV_INCLUDE_DIRS
    REQUIRES driver esp_app_format esp_event console 
    PRIV_REQUIRES command ramses-debug ramses-buttons ramses-led ramses-nvs ramses-network ramses-mqtt cc1101 frame message gateway 
)
