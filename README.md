# Dead Runner Reborn — RP2350-Touch-LCD-3.49

A portable C game-core prototype for a Dead Runner-inspired pseudo-3D runner, targeting the Waveshare RP2350-Touch-LCD-3.49.

## Current status: M0–M3 integrated

- **M0:** portable game core and renderer callback interface.
- **M1:** pseudo-3D scene primitives and lightweight software renderer.
- **M2:** player movement, obstacle collision, game-over and restart behavior.
- **M3:** in-RAM grave records and a normalized tilt-input adapter hook.

## Host tests

GitHub Actions compiles the portable C core with strict warnings and runs smoke and deterministic tests on pushes to `main` and pull requests.

Local command:

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/game \
  src/game/game.c src/game/renderer.c tests/smoke.c tests/host_tests.c \
  -lm -o /tmp/dead-runner-tests
/tmp/dead-runner-tests
```

These tests validate game-core behavior and renderer callbacks only. They do not validate LVGL integration or board hardware.

## Important limitations

This repository is **not yet a complete board firmware project**. It does not currently include Waveshare's board support package, AXS15231B LCD/QSPI initialization, touch driver, QMI8658 sensor driver, an LVGL adapter, or a ready-to-flash UF2. Hardware integration and on-device performance have not yet been verified.

The game core is intentionally separated from LVGL and the vendor BSP. Use the official Waveshare C/LVGL example for this exact board as the hardware foundation, preserving its display, touch, and IMU initialization. Add `src/game/game.c` and `src/game/renderer.c` to that build, then implement the drawing callbacks declared in `src/game/game.h` using the LVGL version shipped with the vendor example.

## Memory and rendering notes

At 640 × 172, an RGB565 full-screen buffer occupies 220,160 bytes (about 215 KiB). Two full-screen buffers would use about 430 KiB before LVGL, stack, and other allocations, so avoid assuming double full-screen buffering will fit in the RP2350's internal SRAM. Prefer partial buffers or drawing through the vendor display pipeline.

The renderer is a compact reference implementation, not yet tuned or benchmarked on the target board.

## Planned next step

Integrate the game renderer with the official Waveshare BSP/LVGL example, then validate screen orientation, frame pacing, touch controls, and QMI8658 tilt input on the actual hardware.
