file(GLOB_RECURSE GxEPD2_SRCS "src/*.cpp")

idf_component_register(
    SRCS ${GxEPD2_SRCS}
    INCLUDE_DIRS "." "src"
    REQUIRES arduino Adafruit_GFX
)
