cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Use the board definition and hardware drivers from the working reference
# firmware. The submodule is pinned to a known commit in the Git tree.
set(PICO_BOARD_HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/vendor/pico-digital-clock/boards")
set(PICO_BOARD waveshare_rp2350_touch_lcd_3.49 CACHE STRING "Pico board")

include(vendor/pico-digital-clock/pico_sdk_import.cmake)

project(pico-death-runner C CXX ASM)
pico_sdk_init()

set(WAVESHARE_BSP "${CMAKE_CURRENT_LIST_DIR}/vendor/pico-digital-clock")
set(QSPI_DIR "${WAVESHARE_BSP}/lib/QSPI_PIO")

add_subdirectory("${WAVESHARE_BSP}/lib/Config" "${CMAKE_CURRENT_BINARY_DIR}/bsp/Config")
add_subdirectory("${QSPI_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/bsp/QSPI_PIO")
add_subdirectory("${WAVESHARE_BSP}/lib/LCD" "${CMAKE_CURRENT_BINARY_DIR}/bsp/LCD")
add_subdirectory("${WAVESHARE_BSP}/lib/Touch" "${CMAKE_CURRENT_BINARY_DIR}/bsp/Touch")
add_subdirectory("${WAVESHARE_BSP}/lib/QMI8658" "${CMAKE_CURRENT_BINARY_DIR}/bsp/QMI8658")

add_executable(pico-death-runner
    src/board/main.c
    src/game/game.c
    src/game/renderer.c
)

pico_generate_pio_header(
    QSPI_PIO
    "${QSPI_DIR}/qspi.pio"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_include_directories(pico-death-runner PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/game"
    "${CMAKE_CURRENT_LIST_DIR}/src/board"
    "${WAVESHARE_BSP}/include"
    "${WAVESHARE_BSP}/lib/Config"
    "${WAVESHARE_BSP}/lib/QSPI_PIO"
    "${WAVESHARE_BSP}/lib/LCD"
    "${WAVESHARE_BSP}/lib/Touch"
    "${WAVESHARE_BSP}/lib/QMI8658"
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_link_libraries(pico-death-runner PRIVATE
    Config
    QSPI_PIO
    LCD
    Touch
    QMI8658
    pico_stdlib
    hardware_pio
    hardware_pwm
    hardware_dma
    hardware_spi
    hardware_i2c
    hardware_gpio
    hardware_adc
    hardware_clocks
)

pico_set_program_name(pico-death-runner "pico-death-runner")
pico_set_program_version(pico-death-runner "0.1")
pico_enable_stdio_uart(pico-death-runner 0)
pico_enable_stdio_usb(pico-death-runner 1)
pico_add_extra_outputs(pico-death-runner)
