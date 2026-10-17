#include "game.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned clears;
    unsigned rects;
    unsigned lines;
} draw_counts_t;

static void count_clear(void *ctx, uint16_t color) {
    draw_counts_t *counts = (draw_counts_t *)ctx;
    (void)color;
    counts->clears++;
}
static void count_rect(void *ctx, int x, int y, int w, int h, uint16_t color) {
    draw_counts_t *counts = (draw_counts_t *)ctx;
    (void)x; (void)y; (void)color;
    assert(w > 0);
    assert(h > 0);
    counts->rects++;
}
static void count_line(void *ctx, int x1, int y1, int x2, int y2, uint16_t color) {
    draw_counts_t *counts = (draw_counts_t *)ctx;
    (void)x1; (void)y1; (void)x2; (void)y2; (void)color;
    counts->lines++;
}

static void test_tilt_deadzone_and_clamp(void) {
    dr_game_t game;
    game_init(&game, 1u);
    game_set_tilt_input(&game, 0.02f, 0.05f, 3.0f);
    assert(game.input_axis == 0.0f);
    game_set_tilt_input(&game, 0.5f, 0.05f, 3.0f);
    assert(game.input_axis > 1.0f - 0.0001f);
    game_set_tilt_input(&game, -0.5f, 0.05f, 3.0f);
    assert(game.input_axis < -1.0f + 0.0001f);
}

static void test_collision_transitions_to_game_over(void) {
    dr_game_t game;
    game_init(&game, 2u);
    game_start(&game);
    for (size_t i = 0; i < DR_MAX_OBJECTS; ++i) game.objects[i].active = false;
    game.objects[0].active = true;
    game.objects[0].type = DR_OBJ_TREE;
    game.objects[0].x = 0.0f;
    game.objects[0].z = 1.0f;
    game.player_x = 0.0f;
    game_update(&game, 100u);
    assert(game.state == DR_STATE_GAME_OVER);
}

static void test_grave_ring_buffer(void) {
    dr_game_t game;
    game_init(&game, 3u);
    for (unsigned i = 0; i < DR_MAX_GRAVES + 1u; ++i)
        game_add_grave(&game, (float)i / 10.0f, (float)i);
    assert(game.grave_count == DR_MAX_GRAVES);
    assert(game.grave_cursor == 1u);
    assert(game.graves[0].x == 1.0f);
    assert(game.graves[0].z == (float)DR_MAX_GRAVES);
}

static void test_renderer_uses_draw_callbacks(void) {
    dr_game_t game;
    draw_counts_t counts;
    memset(&counts, 0, sizeof(counts));
    game_init(&game, 4u);
    game_add_grave(&game, 0.2f, 12.0f);
    dr_draw_api_t draw = {
        .clear = count_clear,
        .rect = count_rect,
        .line = count_line,
        .ctx = &counts,
        .width = DR_SCREEN_W,
        .height = DR_SCREEN_H
    };
    game_render(&game, &draw);
    assert(counts.clears == 1u);
    assert(counts.rects > 0u);
    assert(counts.lines > 0u);
}

int main(void) {
    test_tilt_deadzone_and_clamp();
    test_collision_transitions_to_game_over();
    test_grave_ring_buffer();
    test_renderer_uses_draw_callbacks();
    puts("host tests: PASS");
    return 0;
}
