idf_component_register(SRCS "TimeManager.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES ds1307 clock_logic esp_event esp_netif esp_timer
                       PRIV_REQUIRES esp_wifi nvs_flash)
