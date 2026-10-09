# Lab 2: Hardware I2C Master Driver & LCD (PCF8574 @ 0x27)
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(lab02_i2c_lcd)
