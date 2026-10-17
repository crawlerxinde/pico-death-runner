#include "game.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "DEV_Config.h"
#include "LCD_3in49.h"
#include "QMI8658.h"
#include "Touch.h"
#include "qspi_pio.h"

#define FRAME_INTERVAL_MS 33u
#define TOUCH_POLL_INTERVAL_MS 40u
#define TILT_DEADZONE 0.12f
#define TILT_GAIN 1.8f

/*
 * The panel is physically 172 x 640, while the game renders in landscape
 * 640 x 172 coordinates. Store pixels in the panel's physical row order and
 * rotate each logical drawing operation by 90 degrees. This is one RGB565
 * frame (220,160 bytes), not a pair of full-screen buffers.
 */
static uint16_t lcd_framebuffer[LCD_3IN49_WIDTH * LCD_3IN49_HEIGHT];

static uint16_t swap_rgb565_bytes(uint16_t color)
{
    return (uint16_t)((color << 8) | (color >> 8));
}

static void put_pixel(int x, int y, uint16_t color)
{
    if (x < 0 || x >= DR_SCREEN_W || y < 0 || y >= DR_SCREEN_H) {
        return;
    }

    const int panel_x = y;
    const int panel_y = DR_SCREEN_W - 1 - x;
    lcd_framebuffer[panel_y * LCD_3IN49_WIDTH + panel_x] =
        swap_rgb565_bytes(color);
}

static void draw_clear(void *ctx, uint16_t color)
{
    (void)ctx;
    const uint16_t panel_color = swap_rgb565_bytes(color);
    for (size_t i = 0; i < (size_t)LCD_3IN49_WIDTH * LCD_3IN49_HEIGHT; ++i) {
        lcd_framebuffer[i] = panel_color;
    }
}

static void draw_rect(void *ctx, int x, int y, int w, int h, uint16_t color)
{
    (void)ctx;
    if (w <= 0 || h <= 0) {
        return;
    }

    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w;
    int y1 = y + h;
    if (x1 > DR_SCREEN_W) x1 = DR_SCREEN_W;
    if (y1 > DR_SCREEN_H) y1 = DR_SCREEN_H;
    if (x0 >= x1 || y0 >= y1) {
        return;
    }

    const uint16_t panel_color = swap_rgb565_bytes(color);
    for (int py = y0; py < y1; ++py) {
        const int panel_x = py;
        for (int px = x0; px < x1; ++px) {
            const int panel_y = DR_SCREEN_W - 1 - px;
            lcd_framebuffer[panel_y * LCD_3IN49_WIDTH + panel_x] = panel_color;
        }
    }
}

static void draw_line(void *ctx, int x0, int y0, int x1, int y1, uint16_t color)
{
    (void)ctx;
    /* Integer Bresenham line rasterizer; put_pixel also clips to the canvas. */
    const int dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
    const int sx = (x0 < x1) ? 1 : -1;
    const int dy_abs = (y0 < y1) ? (y1 - y0) : (y0 - y1);
    const int dy = -dy_abs;
    const int sy = (y0 < y1) ? 1 : -1;
    int error = dx + dy;

    for (;;) {
        put_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        const int twice_error = 2 * error;
        if (twice_error >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twice_error <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

static bool touch_was_down;

static void poll_touch(dr_game_t *game)
{
    Touch_Read_State();
    const bool touch_down = TOUCH.Finger_Num > 0;
    if (touch_down && !touch_was_down) {
        if (game->state == DR_STATE_READY) {
            game_start(game);
        } else if (game->state == DR_STATE_GAME_OVER) {
            game_restart(game);
        }
    }
    touch_was_down = touch_down;
}

int main(void)
{
    if (DEV_Module_Init() != 0) {
        printf("Board peripheral initialization failed\r\n");
        return 1;
    }

    QSPI_GPIO_Init(qspi);
    QSPI_PIO_Init(qspi);
    QSPI_4Wrie_Mode(&qspi);
    LCD_3IN49_Init();
    DEV_SET_PWM(60);
    LCD_3IN49_Clear(BLACK);

    (void)QMI8658_init();

    dr_game_t game;
    game_init(&game, 0xD34D1234u);

    const dr_draw_api_t draw = {
        .clear = draw_clear,
        .rect = draw_rect,
        .line = draw_line,
        .ctx = NULL,
        .width = DR_SCREEN_W,
        .height = DR_SCREEN_H
    };

    uint32_t last_frame_ms = to_ms_since_boot(get_absolute_time());
    uint32_t last_touch_ms = last_frame_ms;
    uint32_t last_sensor_ms = last_frame_ms;
    float acc_xyz[3] = {0.0f, 0.0f, 1.0f};

    printf("Dead Runner board firmware ready; tap to start.\r\n");

    for (;;) {
        const uint32_t now_ms = to_ms_since_boot(get_absolute_time());

        if ((uint32_t)(now_ms - last_touch_ms) >= TOUCH_POLL_INTERVAL_MS) {
            poll_touch(&game);
            last_touch_ms = now_ms;
        }

        if ((uint32_t)(now_ms - last_sensor_ms) >= 20u) {
            QMI8658_read_acc_xyz(acc_xyz);
            /*
             * Axis/sign are intentionally isolated here: verify on hardware
             * that tilting left/right changes acc_xyz[0] in the expected
             * direction, then adjust this mapping if the board is rotated.
             */
            game_set_tilt_input(&game, acc_xyz[0], TILT_DEADZONE, TILT_GAIN);
            last_sensor_ms = now_ms;
        }

        if ((uint32_t)(now_ms - last_frame_ms) >= FRAME_INTERVAL_MS) {
            const uint32_t dt_ms = now_ms - last_frame_ms;
            game_update(&game, dt_ms);
            game_render(&game, &draw);

            /*
             * Use DisplayWindows for the full panel: in this BSP revision,
             * Display() configures a DMA length slightly larger than the
             * 172 x 640 framebuffer. DisplayWindows transfers the exact
             * requested dimensions.
             */
            LCD_3IN49_DisplayWindows(
                0, 0, LCD_3IN49_WIDTH, LCD_3IN49_HEIGHT, lcd_framebuffer);
            last_frame_ms = now_ms;
        }

        sleep_ms(1);
    }
}
