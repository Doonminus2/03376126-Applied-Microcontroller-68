idf_component_register(SRCS "dht11.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_rom esp_driver_gpio)
