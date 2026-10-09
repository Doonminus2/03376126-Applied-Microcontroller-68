# Lab 4: Flash File Systems (SPIFFS / VFS)
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(lab04_spiffs)
