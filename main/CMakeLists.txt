idf_component_register(SRCS "blink_example_main.c"
                       INCLUDE_DIRS "."
                       REQUIRES dht11 ultrasonic esp_wifi esp_event esp_netif
                                esp_http_client nvs_flash cjson lcd_hd44780)
