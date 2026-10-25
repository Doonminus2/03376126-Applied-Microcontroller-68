idf_component_register(SRCS "StopwatchEngine.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES clock_logic esp_timer freertos log)
