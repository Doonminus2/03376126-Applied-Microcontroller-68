idf_component_register(SRCS "main.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES i2c_bus ds1307 ssd1306 clock_logic)
