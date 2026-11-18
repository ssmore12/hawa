# Air Monitor — Progress & Learning Log

> **For a new Claude session:** read this file first. The user is here to *learn*, not just to get a finished device.
> Work one task at a time. Before each task: explain the concept briefly, let the user write/attempt the code where reasonable, then review it together.
> After each task: update the checklist, and fill in "Learned" and "Struggled" below. Don't skip ahead without the user's OK.

## About the user & goal
- Senior SWE at Apple; using this project to **build a portfolio and skills for hardware/embedded roles**.
- So: skip hand-holding on general software (git, testing, architecture); spend teaching time on hardware-specific skills.
- Prefer the "how a hardware engineer would do it" path over the quickest library route: read the datasheet, write the driver ourselves, verify with instruments, document it.
- Treat the repo as a portfolio piece: clean history, README, docs, tests, CI.

## Hardware-role practices to weave into every task
- **Datasheet-first:** for each part, read the relevant sections (I2C address, command set, timing, CRC, power) before coding. Record key facts in `docs/`.
- **Write drivers against a thin HAL** (`i2c_bus` interface) so driver logic is host-testable; use vendor libs (e.g. Bosch BME68x) only after understanding what they do.
- **Verify with instruments, not just logs:** logic analyzer / scope (or Saleae-compatible cheap analyzer + sigrok/PulseView) on I2C; multimeter for rails. Capture screenshots into `docs/`.
- **Robustness:** error codes everywhere, bus recovery (stuck SDA), timeouts, task watchdog, brown-out behavior, sensor-failure handling.
- **Engineering hygiene:** Kconfig for pins/thresholds, unity tests (host + target), CI (GitHub Actions with ESP-IDF container), clang-format/clang-tidy.
- **Documentation:** wiring diagram/schematic (KiCad), pin table, power budget, bring-up notes. Stretch: design a custom PCB in KiCad.
- Vocabulary to pick up along the way: pull-ups, bus capacitance, clock stretching, CRC-8, ADC/DAC, DMA, RMT/I2S peripherals, ISR vs. task context, FreeRTOS priorities, low-power modes, OTA, secure boot.

## Project facts
- Board: **ESP32-S3** (esptool confirmed; native USB Serial/JTAG shows up as /dev/ttyACM0). Project was initially set to target `esp32` by mistake — fix with `idf.py set-target esp32s3`. ESP-IDF **6.1.0**, language: C (`main/main.c`)
- Sensors: BME680 (temp/humidity/pressure/gas), SCD41 (CO2/temp/humidity) — both I2C
- Display: 0.9" 4-pin OLED (almost certainly SSD1306, I2C, addr 0x3C; 128x64 or 128x32 — confirm). Shares the I2C bus with the sensors.
- Wiring: I2C SDA = GPIO4, SCL = GPIO5 (assumed shared by OLED, BME680, SCD41 — confirm). Expected addrs: OLED 0x3C, SCD41 0x62, BME680 0x76/0x77 (no conflicts).
- Equipment: multimeter ZOYI ZT109 (no scope/logic analyzer yet; a ~$10 USB logic analyzer clone + PulseView is a worthwhile buy for I2C debugging)
- Dev env: WSL2 on Linux. USB flashing from WSL needs usbipd (see Task 1 notes).

## Task list
- [x] **0. Hardware inventory** — confirm shared bus, power (3.3V/5V) for each part, check rails + pull-ups with multimeter
- [x] **1. Toolchain** — build, flash, and monitor a hello-world (`idf.py build flash monitor`); get USB visible from WSL2
- [x] **2. GPIO / blink** — done via the Espressif blink example
- [x] **3. I2C bus scan** (all three found: OLED 0x3C, SCD41 0x62, BME680 at 0x77 — SDO pulled high on this clone) — init the I2C master driver, scan, confirm both sensors appear (BME680 ~0x76/0x77, SCD41 0x62)
- [ ] **4. SCD41** — read CO2/temp/humidity (learn: datasheet commands, CRC, measurement timing)
- [ ] **5. BME680** — read temp/humidity/pressure/gas (decide: Bosch BSEC/BME68x driver vs. raw gas resistance)
- [ ] **6. Structure the code** — split into components, sensor task + shared data (queue/mutex)
- [ ] **7. OLED output** — SSD1306 driver over I2C (init sequence, framebuffer, font), show readings + CO2 good/ok/bad
- [ ] **8. Test & CI** — Unity host tests for drivers (CRC, parsing), on-target tests, GitHub Actions build
- [ ] **9. Robustness** — bus recovery, watchdogs, sensor-failure handling, calibration/altitude compensation
- [ ] **10. Connectivity & power (optional)** — Wi-Fi + MQTT or web dashboard, OTA, deep-sleep power measurement
- [ ] **11. Hardware docs / PCB (stretch)** — KiCad schematic of the breadboard, then a custom PCB; README with photos and scope captures

## Learned
- 2026-11-16: Flashing from WSL2 already works (ran the Espressif blink example).

- 2026-11-16: Multimeter basics: DC volts, continuity (unpowered only), resistance. Breadboard rails/columns. Never force probes into breadboard holes — use a jumper wire as an adapter.
- 2026-11-16: I2C pull-ups on modules connect to the module's own VCC, so an unpowered module makes SDA/SCL float. Idle I2C lines should read ~3.3V.
- 2026-11-16: I2C master driver: `i2c_new_master_bus(&cfg, &handle)`, `i2c_master_probe(bus, addr, timeout_ms)` returns ESP_OK on ACK. I2C controller number is arbitrary because of the GPIO matrix. BME680 address depends on the SDO pin (0x76 low / 0x77 high); this clone is 0x77.
- 2026-11-16: Task 0 result: all rails 3.3V, SDA/SCL idle at 3.3V (modules have pull-ups), no shorts.

## Struggled / gotchas
- 2026-11-16: Flash failed with "This chip is ESP32-S3, not ESP32". Project target didn't match the real chip. Fix: `idf.py set-target esp32s3` (regenerates sdkconfig). Lesson: verify chip with `esptool chip-id` before choosing a target.
- 2026-11-16: SDA/SCL read "floating" at first. Root cause was a bad/misplaced jumper from ESP32 3V3 to the + rail (no power to modules), not missing pull-ups. Fix: found with continuity from the 3V3 pin outward. Lesson: debug top-down along the power path before suspecting the signal lines.

## Session log
- 2026-11-16: Project is a fresh ESP-IDF template (`main/main.c` only). Created this plan. Nothing built yet.
