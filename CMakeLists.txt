cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(epaper_bw)

spiffs_create_partition_image(spiffs data FLASH_IN_PROJECT)
