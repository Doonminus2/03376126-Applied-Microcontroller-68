idf_component_register(SRCS "ButtonManager.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES clock_logic esp_driver_gpio freertos
                       PRIV_REQUIRES esp_timer)
