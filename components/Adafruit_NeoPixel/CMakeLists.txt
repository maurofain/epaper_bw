set(ADAFRUIT_NEOPIXEL_SRCS
    "Adafruit_NeoPixel.cpp"
    "esp.c"
)

idf_component_register(
    SRCS ${ADAFRUIT_NEOPIXEL_SRCS}
    INCLUDE_DIRS "."
    REQUIRES arduino
)
