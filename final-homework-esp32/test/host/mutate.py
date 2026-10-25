#!/usr/bin/env python3
"""Mutation check: ใส่บั๊กทีละจุดลงใน "สำเนา" ของ clock_logic/include แล้วรัน host tests

ไฟล์จริงใน components/ ไม่ถูกแก้ ทุก mutant ต้องทำให้มีเทสต์ fail อย่างน้อยหนึ่งตัว
ไม่งั้นสคริปต์จะ exit 1 พร้อมบอกว่า mutant ไหนรอด

    python3 test/host/mutate.py [--workdir DIR]
"""
import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

HOST_DIR = os.path.dirname(os.path.abspath(__file__))
REAL_INCLUDE = os.path.join(HOST_DIR, "..", "..", "components", "clock_logic", "include")

# (ชื่อ, ไฟล์, ข้อความเดิม, ข้อความที่กลายพันธุ์)
MUTATIONS = [
    ("bcd: swapped nibbles in bcdToDec", "bcd.hpp",
     "(bcd >> 4) * 10 + (bcd & 0x0F)", "(bcd & 0x0F) * 10 + (bcd >> 4)"),
    ("bcd: isValidBcd accepts A..F in low nibble", "bcd.hpp",
     "(bcd & 0x0F) <= 9", "(bcd & 0x0F) <= 0x0F"),
    ("ds1307: decode keeps CH bit in seconds", "datetime.hpp",
     "raw[0] & static_cast<uint8_t>(~DS1307_CH_BIT)", "raw[0]"),
    ("ds1307: 12h decode without % 12 (12 AM -> 12)", "datetime.hpp",
     "h12 % 12 +", "h12 +"),
    ("ds1307: encode sets 12h bit", "datetime.hpp",
     "raw[2] = decToBcd(dt.hour);", "raw[2] = decToBcd(dt.hour) | DS1307_12H_BIT;"),
    ("epoch: weekday offset off by one", "datetime.hpp",
     "+ 7 + 4) % 7", "+ 7 + 3) % 7"),
    ("epoch: leap year ignores century rule", "datetime.hpp",
     "(y % 4 == 0 && y % 100 != 0) || y % 400 == 0", "y % 4 == 0"),
    ("epoch: fromEpoch skips range check", "datetime.hpp",
     "    if (!isValid(dt)) return false;\n    out = dt;\n    return true;\n}\n\n// ----",
     "    out = dt;\n    return true;\n}\n\n// ----"),
    ("alarm: compare uses hour only", "alarm_logic.hpp",
     "a.hour == hour && a.minute == minute", "a.hour == hour"),
    ("alarm: no once-per-minute guard", "alarm_logic.hpp",
     "if (!armedForMatch_) return AlarmEvent::None;", ""),
    ("alarm: ring timeout off by one (> instead of >=)", "alarm_logic.hpp",
     "nowMs - startedMs_ >= ringMs_", "nowMs - startedMs_ > ringMs_"),
    ("alarm: disabling does not stop ringing", "alarm_logic.hpp",
     "!a.enabled || nowMs", "nowMs"),
    ("stopwatch: lap split uses total", "stopwatch_core.hpp",
     "splitMs = total - lastLapTotalMs_", "splitMs = total"),
    ("stopwatch: lap capacity one short", "stopwatch_core.hpp",
     "lapCount_ >= STOPWATCH_MAX_LAPS", "lapCount_ >= STOPWATCH_MAX_LAPS - 1"),
    ("stopwatch: reset keeps previous lap total", "stopwatch_core.hpp",
     "        lastLapTotalMs_ = 0;\n", ""),
    ("stopwatch: start while running restarts", "stopwatch_core.hpp",
     "if (running_) return;", ""),
    ("stopwatch: stop forgets accumulated time", "stopwatch_core.hpp",
     "accumulatedUs_ += nowUs - startedAtUs_;", "accumulatedUs_ = nowUs - startedAtUs_;"),
    ("button: debounce ignored", "button_fsm.hpp",
     "nowMs - rawChangedMs_ >= debounceMs_", "true"),
    ("button: long-press threshold off by one", "button_fsm.hpp",
     "nowMs - pressedAtMs_ >= longPressMs_", "nowMs - pressedAtMs_ > longPressMs_"),
    ("button: Short also fires after Long", "button_fsm.hpp",
     "} else if (!longReported_) {", "} else {"),
    ("button: idle ignores pending long press", "button_fsm.hpp",
     "raw_ == stable_ && (!stable_ || longReported_)", "raw_ == stable_"),
    ("ui: mode order swapped (Alarm <-> Stopwatch)", "ui_mode.hpp",
     "{ Clock, SetTime, Alarm, Stopwatch }", "{ Clock, SetTime, Stopwatch, Alarm }"),
    ("ui: modeName typo", "ui_mode.hpp",
     'return "SET TIME";', 'return "SETTIME";'),
    ("ui: SET long does not wrap field", "ui_mode.hpp",
     "(ui.field + 1) % fieldCount(ui.mode)", "ui.field + 1"),
    ("ui: MODE press keeps field", "ui_mode.hpp",
     "                ui.field = 0;\n", ""),
    ("ui: LAP long behaves like short", "ui_mode.hpp",
     ": Command::StopwatchReset", ": Command::StopwatchLapOrReset"),
    ("ui: ALARM toggles only in Clock mode", "ui_mode.hpp",
     "if (press == PressType::Short) r.command = Command::ToggleAlarm;",
     "if (press == PressType::Short && ui.mode == Mode::Clock) r.command = Command::ToggleAlarm;"),
    ("ui: wrapIncrement wraps one too late", "ui_mode.hpp",
     "(value + 1) % modulus", "(value + 1) % (modulus + 1)"),
]

FAIL_LINE = re.compile(r"^[^\n]*?:\d+:(test_\w+):FAIL", re.MULTILINE)


def run_suite(include_dir, build_dir, cxxflags=""):
    env = dict(os.environ, CLOCK_LOGIC_INCLUDE=include_dir, HOST_BUILD_DIR=build_dir, HOST_CXXFLAGS=cxxflags)
    proc = subprocess.run([os.path.join(HOST_DIR, "run.sh")], env=env, capture_output=True, text=True)
    return proc.returncode, proc.stdout + proc.stderr


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--workdir", default=None)
    args = parser.parse_args()
    workdir = tempfile.mkdtemp(prefix="clock_logic_mutants_", dir=args.workdir)

    code, out = run_suite(os.path.abspath(REAL_INCLUDE), os.path.join(workdir, "baseline"))
    if code != 0:
        print(out)
        sys.exit("baseline suite must pass before mutation testing")

    survivors = []
    print("| # | Mutation | Tests that failed |")
    print("|---|---|---|")
    for i, (name, header, before, after) in enumerate(MUTATIONS, 1):
        mutant_root = os.path.join(workdir, f"m{i:02d}")
        include_copy = os.path.join(mutant_root, "include")
        shutil.copytree(REAL_INCLUDE, include_copy)
        path = os.path.join(include_copy, "clock_logic", header)
        with open(path) as f:
            src = f.read()
        if src.count(before) != 1:
            sys.exit(f"mutation '{name}': pattern must occur exactly once in {header}, found {src.count(before)}")
        with open(path, "w") as f:
            f.write(src.replace(before, after))

        code, out = run_suite(include_copy, os.path.join(mutant_root, "build"), "-Wno-error")
        failed = sorted(set(FAIL_LINE.findall(out)))
        if code != 0 and failed:
            print(f"| {i} | {name} | {', '.join(failed)} |")
        elif code != 0:
            survivors.append(name)
            print(f"| {i} | {name} | BUILD/RUN ERROR, no test verdict |")
            print(out[-2000:], file=sys.stderr)
        else:
            survivors.append(name)
            print(f"| {i} | {name} | SURVIVED |")

    print(f"\n{len(MUTATIONS) - len(survivors)}/{len(MUTATIONS)} mutants killed (work dir {workdir})")
    sys.exit(1 if survivors else 0)


if __name__ == "__main__":
    main()
