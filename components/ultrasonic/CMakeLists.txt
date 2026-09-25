idf_component_register(SRCS "ultrasonic.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver esp_rom esp_timer esp_driver_gpio)
