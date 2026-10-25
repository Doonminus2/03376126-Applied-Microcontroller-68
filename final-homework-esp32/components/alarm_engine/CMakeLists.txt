idf_component_register(SRCS "AlarmEngine.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES clock_logic esp_driver_ledc esp_driver_gpio esp_timer freertos)
