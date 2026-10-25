# Spiral 3: Input + UI (ButtonManager debounce, short/long press, โหมด UI บน SSD1306)
cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/../components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(final03_input_ui)
