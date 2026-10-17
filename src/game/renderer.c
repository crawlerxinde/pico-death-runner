#include "game.h"
#include <math.h>
#include <stdio.h>

#define C_BLACK 0x0000u
#define C_FOG   0x3186u
#define C_TREE  0x632Cu
#define C_LIGHT 0xBDF7u
#define C_GROUND 0x2104u
#define C_ACCENT 0xFCA0u

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static void rect(const dr_draw_api_t *d, int x, int y, int w, int h, uint16_t c) {
    if (!d || !d->rect || w <= 0 || h <= 0) return;
    d->rect(d->ctx, x, y, w, h, c);
}

/* Compact 3x5 bitmap font: A-Z followed by 0-9, with 3-bit rows. */
static const uint8_t FONT_3X5[36][5] = {
    {2,5,7,5,5}, {6,5,6,5,6}, {3,4,4,4,3}, {6,5,5,5,6},
    {7,4,6,4,7}, {7,4,6,4,4}, {3,4,5,5,3}, {5,5,7,5,5},
    {7,2,2,2,7}, {1,1,1,5,2}, {5,5,6,5,5}, {4,4,4,4,7},
    {5,7,7,5,5}, {5,7,7,7,5}, {2,5,5,5,2}, {6,5,6,4,4},
    {2,5,5,3,1}, {6,5,6,5,5}, {3,4,2,1,6}, {7,2,2,2,2},
    {5,5,5,5,7}, {5,5,5,5,2}, {5,5,7,7,5}, {5,5,2,5,5},
    {5,5,2,2,2}, {7,1,2,4,7},
    {2,5,5,5,2}, {2,3,2,2,7}, {6,1,2,4,7}, {6,1,2,1,6},
    {5,5,7,1,1}, {7,4,6,1,6}, {3,4,6,5,2}, {7,1,2,2,2},
    {2,5,2,5,2}, {2,5,3,1,6}
};

static uint8_t glyph_row(char ch, int row) {
    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    if (ch >= 'A' && ch <= 'Z') return FONT_3X5[ch - 'A'][row];
    if (ch >= '0' && ch <= '9') return FONT_3X5[26 + ch - '0'][row];
    return 0u;
}

static void draw_text(const dr_draw_api_t *d, int x, int y, int scale,
                      const char *text, uint16_t color) {
    if (!text || scale < 1) return;
    for (const char *p = text; *p; ++p, x += 4 * scale) {
        if (*p == ' ') continue;
        for (int row = 0; row < 5; ++row) {
            const uint8_t bits = glyph_row(*p, row);
            for (int col = 0; col < 3; ++col) {
                if (bits & (uint8_t)(1u << (2 - col)))
                    rect(d, x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

static void draw_hud(const dr_game_t *g, const dr_draw_api_t *d) {
    char line[20];
    (void)snprintf(line, sizeof(line), "SCORE %06lu", (unsigned long)g->score);
    draw_text(d, 8, 7, 2, line, C_LIGHT);
    (void)snprintf(line, sizeof(line), "BEST  %06lu", (unsigned long)g->best_score);
    draw_text(d, 8, 21, 2, line, C_ACCENT);
}

void game_render(const dr_game_t *g, const dr_draw_api_t *d) {
    if (!g || !d || d->width <= 0 || d->height <= 0) return;
    if (d->clear) d->clear(d->ctx, C_BLACK);
    const int w = d->width, h = d->height;
    const int horizon = h * 34 / 100;
    rect(d, 0, horizon, w, h - horizon, C_GROUND);
    for (int i = 0; i < 4; ++i) rect(d, 0, horizon + i * 5, w, 2, C_FOG);
    if (d->line) {
        for (int i = 1; i <= 5; ++i) {
            int y = horizon + (h - horizon) * i * i / 36;
            d->line(d->ctx, 0, y, w - 1, y, C_FOG);
        }
    }
    const float focal = (float)w * 0.28f;
    for (size_t i = 0; i < DR_MAX_OBJECTS; ++i) {
        const dr_object_t *o = &g->objects[i];
        if (!o->active || o->z < 1.0f) continue;
        const float scale = focal / o->z;
        const int sx = (int)((float)w * 0.5f + o->x * focal);
        const int base_y = horizon + (int)((float)(h - horizon) * (1.0f - 1.0f / (1.0f + 18.0f / o->z)));
        const int ow = clampi((int)(scale * 0.58f), 2, w / 3);
        const int oh = clampi((int)(scale * 1.7f), 3, h * 2);
        if (sx + ow < 0 || sx - ow >= w || base_y < 0 || base_y >= h) continue;
        rect(d, sx - ow / 8, base_y - oh / 3, ow / 4 + 1, oh / 3 + 1, C_TREE);
        rect(d, sx - ow / 2, base_y - oh / 2, ow, oh / 4 + 1, C_TREE);
        rect(d, sx - ow / 3, base_y - oh * 3 / 4, ow * 2 / 3, oh / 3 + 1, C_TREE);
        rect(d, sx - ow / 6, base_y - oh, ow / 3 + 1, oh / 3 + 1, C_TREE);
    }
    for (size_t i = 0; i < g->grave_count; ++i) {
        const dr_grave_t *grave = &g->graves[i];
        if (grave->z < 1.0f) continue;
        const float scale = focal / grave->z;
        const int sx = (int)((float)w * 0.5f + grave->x * focal);
        const int gy = horizon + (int)((float)(h - horizon) * (1.0f - 1.0f / (1.0f + 18.0f / grave->z)));
        const int gw = clampi((int)(scale * 0.24f), 2, w / 12);
        const int gh = clampi((int)(scale * 0.48f), 3, h / 2);
        rect(d, sx - gw / 2, gy - gh, gw, gh, C_LIGHT);
        rect(d, sx - gw, gy - gh, gw * 2, gh / 4 + 1, C_LIGHT);
    }
    const int px = w / 2 + (int)(g->player_x * (float)w * 0.34f);
    const int py = h - 9;
    rect(d, px - 2, py - 13, 5, 5, C_LIGHT);
    rect(d, px - 3, py - 8, 7, 6, C_LIGHT);
    rect(d, px - 4, py - 3, 3, 3, C_LIGHT);
    rect(d, px + 1, py - 3, 3, 3, C_LIGHT);

    draw_hud(g, d);
    if (g->state == DR_STATE_READY) {
        const char *prompt = "TAP TO START";
        draw_text(d, (w - 12 * 4 * 4) / 2, h / 2 - 12, 4, prompt, C_LIGHT);
    } else if (g->state == DR_STATE_GAME_OVER) {
        draw_text(d, (w - 9 * 4 * 4) / 2, h / 2 - 18, 4, "GAME OVER", C_ACCENT);
        draw_text(d, (w - 12 * 4 * 3) / 2, h / 2 + 5, 3, "TAP TO RETRY", C_LIGHT);
    }
}
