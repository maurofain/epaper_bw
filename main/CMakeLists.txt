idf_component_register(
    SRCS "../src/main.cpp"
         "../src/led_control.cpp"
         "../src/scanner_control.cpp"
         "../src/master_protocol.cpp"
    INCLUDE_DIRS "../include"
    REQUIRES arduino
)
