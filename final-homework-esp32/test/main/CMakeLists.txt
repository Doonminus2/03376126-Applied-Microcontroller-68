idf_component_register(SRCS "main.cpp" "run_all.cpp"
                            "test_bcd.cpp" "test_datetime.cpp" "test_alarm.cpp"
                            "test_stopwatch.cpp" "test_button.cpp" "test_ui.cpp"
                       INCLUDE_DIRS "."
                       REQUIRES unity clock_logic)
