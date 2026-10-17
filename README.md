# Dead Runner Reborn — RP2350-Touch-LCD-3.49

A portable C game-core prototype for a Dead Runner-inspired pseudo-3D runner,
targeting the Waveshare RP2350-Touch-LCD-3.49.

## Current status

- **M0:** portable game core and renderer callback interface.
- **M1:** pseudo-3D scene primitives and lightweight software renderer.
- **M2:** player movement, obstacle collision, game-over and restart behavior.
- **M3:** in-RAM grave records and normalized tilt-input adapter hook.
- **M4 bring-up:** board firmware target using the working
  [pico-digital-clock](https://github.com/crawlerxinde/pico-digital-clock)
  hardware drivers as a pinned submodule; landscape renderer adapter, touch
  start/restart, and QMI8658 tilt input are wired in.

## Host tests

GitHub Actions compiles the portable C core with strict warnings and runs smoke
and deterministic tests on pushes to `main` and pull requests.

Local command:

```sh
cc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc/game \
  src/game/game.c src/game/renderer.c tests/smoke.c tests/host_tests.c \
  -lm -o /tmp/dead-runner-tests
/tmp/dead-runner-tests
```

These tests validate game-core behavior and renderer callbacks. They do not
validate on-device orientation, touch/IMU axis mapping, or frame rate.

## Build board firmware

Install Pico SDK 2.3.1 or newer, CMake, Ninja, and the ARM embedded toolchain.
Initialize the board-support submodule:

```sh
git submodule update --init --recursive
export PICO_SDK_PATH="$HOME/.pico-sdk/sdk/2.3.1"
cmake -S . -B build -G Ninja \
  -DPICO_SDK_PATH="$PICO_SDK_PATH" \
  -DPICO_BOARD=waveshare_rp2350_touch_lcd_3.49
cmake --build build
```

The resulting firmware is `build/pico-death-runner.uf2`. GitHub Actions also
builds the board target on pull requests.

## Board bring-up notes

- Physical panel: 172 × 640; logical game canvas: 640 × 172.
- A single 172 × 640 RGB565 framebuffer is rotated into physical panel order
  (220,160 bytes), avoiding two full-screen buffers in internal SRAM.
- Tap to start and tap after a collision to restart.
- Tilt steering reads the QMI8658 accelerometer; its axis and sign need
  verification on the actual board.
- The board CI build verifies compilation and UF2 generation, not physical
  display orientation, colors, input response, or frame pacing.

See [src/board/README.md](src/board/README.md) for board-target details.
