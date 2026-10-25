idf_component_register(SRCS "DS1307.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES i2c_bus clock_logic esp_driver_i2c)
