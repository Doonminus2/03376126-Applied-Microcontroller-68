# Wokwi ESP32 LED Blink Integration Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the existing ESP-IDF project simulate one LED connected from GPIO17 to GND and blink it once per second in Wokwi for VS Code.

**Architecture:** Keep the existing ESP-IDF project and component structure. Describe one official ESP32 DevKitC V4 board and one LED in `diagram.json`, load the complete ESP-IDF flash layout through `build/flasher_args.json`, and retain the ELF for symbols.

**Tech Stack:** ESP-IDF v6.0.2, FreeRTOS, Wokwi for VS Code 3.6.0, JSON, TOML

## Global Constraints

- Keep the ESP-IDF framework; do not switch to Arduino.
- Use GPIO17 and one-second ON/OFF delays via `vTaskDelay()`.
- Do not create a new project or remove unrelated ESP-IDF configuration.
- Validate the JSON, build the project, and verify every referenced build artifact.

---

### Task 1: Establish the failing integration check

**Files:**
- Inspect: `diagram.json`
- Inspect: `wokwi.toml`
- Inspect: `main/blink_example_main.c`

**Interfaces:**
- Consumes: Existing project configuration and build output.
- Produces: A concrete check for the official board pin IDs, ESP-IDF flash manifest, and GPIO timing behavior.

- [x] **Step 1: Run a read-only assertion for the target configuration**

Use a Node.js one-off assertion that requires `board-esp32-devkit-c-v4`, `esp:17`, `led:A`, `led:C`, `esp:GND.1`, `build/flasher_args.json`, `build/blink.elf`, GPIO17, and two 1000 ms delays.

- [x] **Step 2: Confirm that the current configuration fails for the expected board/firmware differences**

Expected: FAIL because the current diagram uses the unofficial board/pin naming and `wokwi.toml` points only at the application binary.

### Task 2: Apply the minimal Wokwi and blink configuration

**Files:**
- Modify: `diagram.json`
- Modify: `wokwi.toml`
- Modify only if behavior differs: `main/blink_example_main.c`

**Interfaces:**
- Consumes: Wokwi board pin `17`, ground `GND.1`, LED `A`/`C`, and ESP-IDF build metadata.
- Produces: A two-wire LED circuit and complete ESP-IDF firmware loading configuration.

- [x] **Step 1: Replace the diagram with one official ESP32 DevKitC V4 and one LED**

Connect `esp:17` to `led:A` and `led:C` to `esp:GND.1`, with visible wire colors and explicit horizontal routing.

- [x] **Step 2: Point Wokwi at ESP-IDF build outputs**

Set `firmware = 'build/flasher_args.json'` and `elf = 'build/blink.elf'`.

- [x] **Step 3: Keep or minimally adjust the ESP-IDF blink loop**

Require `GPIO_NUM_17`, output mode, HIGH then LOW, and `vTaskDelay(pdMS_TO_TICKS(1000))` after each state.

- [x] **Step 4: Re-run the target assertion**

Expected: PASS.

### Task 3: Build and verify artifacts

**Files:**
- Verify: `build/flasher_args.json`
- Verify: `build/blink.bin`
- Verify: `build/blink.elf`

**Interfaces:**
- Consumes: The existing ESP-IDF v6.0.2 environment.
- Produces: Fresh binaries and evidence that Wokwi references valid files.

- [x] **Step 1: Run `idf.py build`**

Expected: Exit code 0 and project `blink` built successfully.

- [x] **Step 2: Parse `diagram.json` and `build/flasher_args.json`**

Expected: Valid JSON; every flash image listed by `flasher_args.json` exists.

- [x] **Step 3: Parse `wokwi.toml` paths and verify artifacts**

Expected: `build/flasher_args.json`, `build/blink.bin`, and `build/blink.elf` exist and are non-empty.

- [x] **Step 4: Run the complete target assertion again**

Expected: PASS with no failed checks.
