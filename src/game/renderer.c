#include "game.h"
#include <math.h>

#define C_BLACK 0x0000u
#define C_FOG   0x3186u
#define C_TREE  0x632Cu
#define C_LIGHT 0xBDF7u
#define C_GROUND 0x2104u

static int clampi(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static void rect(const dr_draw_api_t *d, int x, int y, int w, int h, uint16_t c) {
    if (!d || !d->rect || w <= 0 || h <= 0) return;
    d->rect(d->ctx, x, y, w, h, c);
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
}
