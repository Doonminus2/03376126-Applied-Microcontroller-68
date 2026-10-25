idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES i2c_bus ds1307 ssd1306 clock_logic time_manager button_manager alarm_engine
                                stopwatch_engine esp_timer freertos)
