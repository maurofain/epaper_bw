idf_component_register(
    SRCS "../src/main.cpp"
         "../src/led_control.cpp"
         "../src/scanner_control.cpp"
         "../src/master_protocol.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES lvgl esp_wifi esp_netif esp_event esp_driver_uart esp_driver_gpio nvs_flash esp_http_server app_update
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE SCANNER_CONTROL_USE_SERIAL)
