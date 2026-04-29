idf_component_register(
    SRCS "../src/main.cpp"
         "../src/led_control.cpp"
         "../src/scanner_control.cpp"
         "../src/master_protocol.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES GDEY0154D67 lvgl esp_wifi esp_netif esp_event esp_driver_uart esp_driver_gpio esp_driver_spi nvs_flash esp_http_server app_update spiffs
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE SCANNER_CONTROL_USE_SERIAL)
