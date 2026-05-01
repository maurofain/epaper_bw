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
         "../components/lvgl/GoogleSansBold40.c"
         "../components/lvgl/GoogleSansBold60.c"
         "../components/lvgl/GoogleSansBold100.c"
         "../components/lvgl/GoogleSansBold140.c"
    INCLUDE_DIRS "../include"
    REQUIRES GDEY0154D67 lvgl esp_driver_uart esp_driver_gpio esp_driver_spi nvs_flash spiffs app_update esp_wifi esp_netif esp_event esp_http_server
)

# ============================================================================
# Conditional Compilation Flags
# ============================================================================
# All features use 0/1 logic for consistency
# Set to 0 to disable, 1 to enable

# Serial scanner control (enabled by default)
target_compile_definitions(${COMPONENT_LIB} PRIVATE SCANNER_CONTROL_USE_SERIAL=1)

# WiFi and OTA support (DISABLED by default to save ~50KB DIRAM)
# Set to 1 to enable WiFi, event handlers, and OTA HTTP server
target_compile_definitions(${COMPONENT_LIB} PRIVATE USE_WIFI=0)

# USB console for master commands (disabled by default)
# Set to 1 to enable USB console instead of UART for master protocol
target_compile_definitions(${COMPONENT_LIB} PRIVATE MASTER_PROTOCOL_USE_USB_CONSOLE=1)

# Enable logging (enabled for debugging)
# Set to 1 to enable log output via ESP_LOG functions
target_compile_definitions(${COMPONENT_LIB} PRIVATE ENABLE_LOG=1)

# Display test mode (disabled by default)
# Set to 1 to enable display test sequence on startup
target_compile_definitions(${COMPONENT_LIB} PRIVATE ENABLE_DISPLAY_TEST=0)

# ============================================================================
