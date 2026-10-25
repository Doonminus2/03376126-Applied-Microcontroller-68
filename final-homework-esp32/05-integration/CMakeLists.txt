# Spiral 5: รวมระบบ FreeRTOS ทั้งหมด (นาฬิกา + Wi-Fi/SNTP + alarm + stopwatch)
cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/../components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(final05_integration)
