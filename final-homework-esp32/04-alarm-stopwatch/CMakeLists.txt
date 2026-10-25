# Spiral 4: Alarm + Stopwatch (AlarmEngine + บัซเซอร์ LEDC, StopwatchEngine + lap)
cmake_minimum_required(VERSION 3.22)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_LIST_DIR}/../components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)
project(final04_alarm_stopwatch)
