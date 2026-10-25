idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES i2c_bus ssd1306 button_manager clock_logic)
