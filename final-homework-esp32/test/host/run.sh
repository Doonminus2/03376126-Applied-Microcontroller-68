#!/usr/bin/env bash
# รันชุดเทสต์ clock_logic บนเครื่อง host (macOS/Linux) ด้วยซอร์ส Unity ของ ESP-IDF
#   CLOCK_LOGIC_INCLUDE  ชี้ไปยัง include dir อื่น (ใช้ตอนทดลอง mutation)
#   HOST_BUILD_DIR       ที่เก็บไฟล์ build (ค่าเริ่มต้น test/host/build)
#   HOST_CXXFLAGS        flag เพิ่มเติม (mutate.py ใช้ -Wno-error ให้ mutant ถูกตัดสินด้วยเทสต์ ไม่ใช่ warning)
set -euo pipefail

HOST_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_DIR="$(dirname "$HOST_DIR")"
PROJECT_DIR="$(dirname "$TEST_DIR")"

if [ -z "${IDF_PATH:-}" ]; then
    IDF_PATH="$(sh "$HOME/.espressif/tools/activate_idf_v6.0.2.sh" -e | sed -n 's/^IDF_PATH=//p')"
fi

UNITY_SRC="$IDF_PATH/components/unity/unity/src"
CLOCK_LOGIC_INCLUDE="${CLOCK_LOGIC_INCLUDE:-$PROJECT_DIR/components/clock_logic/include}"
BUILD_DIR="${HOST_BUILD_DIR:-$HOST_DIR/build}"
mkdir -p "$BUILD_DIR"

cc -std=c11 -O1 -I"$UNITY_SRC" -c "$UNITY_SRC/unity.c" -o "$BUILD_DIR/unity.o"
c++ -std=c++17 -O1 -Wall -Wextra -Werror ${HOST_CXXFLAGS:-} \
    -I"$UNITY_SRC" -I"$CLOCK_LOGIC_INCLUDE" -I"$TEST_DIR/main" \
    "$TEST_DIR"/main/test_*.cpp "$TEST_DIR/main/run_all.cpp" "$HOST_DIR/host_main.cpp" \
    "$BUILD_DIR/unity.o" -o "$BUILD_DIR/clock_logic_tests"

"$BUILD_DIR/clock_logic_tests"
