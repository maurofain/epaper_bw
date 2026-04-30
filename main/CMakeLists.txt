idf_component_register(
    SRCS "../src/main.cpp"
         "../src/ui/logo.c"
         "../src/led_control.cpp"
         "../src/scanner_control.cpp"
         "../src/master_protocol.cpp"
         "../components/lvgl/GoogleSans10.c"
         "../components/lvgl/GoogleSans15.c"
         "../components/lvgl/GoogleSans20.c"
         "../components/lvgl/GoogleSans35.c"
         "../components/lvgl/GoogleSans50.c"
         "../components/lvgl/GoogleSans60.c"
         "../components/lvgl/GoogleSans100.c"
         "../components/lvgl/GoogleSans140.c"
    INCLUDE_DIRS "../include"
    REQUIRES GDEY0154D67 lvgl esp_wifi esp_netif esp_event esp_driver_uart esp_driver_gpio esp_driver_spi nvs_flash esp_http_server app_update spiffs
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE SCANNER_CONTROL_USE_SERIAL)

if(MASTER_PROTOCOL_USE_USB_CONSOLE)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE MASTER_PROTOCOL_USE_USB_CONSOLE)
endif()
