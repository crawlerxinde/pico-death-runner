# RP2350-Touch-LCD-3.49 board target

The board target reuses the tested Waveshare drivers from the
[pico-digital-clock reference project](https://github.com/crawlerxinde/pico-digital-clock),
pinned as a Git submodule. The game core remains portable and independent of the
board support package.

## Build

Initialize the submodule and use Pico SDK 2.3.1 or newer:

```sh
git submodule update --init --recursive
export PICO_SDK_PATH="$HOME/.pico-sdk/sdk/2.3.1"
cmake -S . -B build -G Ninja \
  -DPICO_SDK_PATH="$PICO_SDK_PATH" \
  -DPICO_BOARD=waveshare_rp2350_touch_lcd_3.49
cmake --build build
```

The firmware is written to `build/pico-death-runner.uf2`.

## Current bring-up behavior

- Tap the touch panel to start; tap again after a collision to restart.
- The QMI8658 accelerometer is sampled for left/right tilt input.
- Game drawing uses a single 172 x 640 RGB565 framebuffer, rotated from the
  game's 640 x 172 logical landscape coordinates.
- The BSP's full-screen `LCD_3IN49_DisplayWindows` path is used to transfer
  exactly the framebuffer dimensions.

The touch panel and IMU axes, RGB565 byte order, and physical rotation still
need confirmation on the actual device. The board CI job verifies compilation
and UF2 generation, not on-device behavior or frame rate.
