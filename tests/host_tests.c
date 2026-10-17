#include "game.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int smoke_test(void);

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
    assert(game.input_axis < 1.0f + 0.0001f && game.input_axis > 0.999f);
    game_set_tilt_input(&game, -0.5f, 0.05f, 3.0f);
    assert(game.input_axis > -1.0f - 0.0001f && game.input_axis < -0.999f);
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
    assert(game.grave_count == 1u);
    assert(game.graves[0].x == game.player_x);
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

static void test_restart_preserves_best_and_graves(void) {
    dr_game_t game;
    game_init(&game, 5u);
    game_start(&game);
    game_update(&game, 100u);
    game_update(&game, 100u);
    const uint32_t best = game.best_score;
    game_add_grave(&game, -0.25f, 7.0f);
    const size_t grave_count = game.grave_count;
    game_restart(&game);
    assert(game.state == DR_STATE_RUNNING);
    assert(game.score == 0u);
    assert(game.best_score == best);
    assert(game.grave_count == grave_count);
    assert(game.graves[grave_count - 1u].x == -0.25f);
    assert(game.speed >= 14.0f);
}

static void test_speed_ramps_and_caps(void) {
    dr_game_t game;
    game_init(&game, 6u);
    game_start(&game);
    game_update(&game, 100u);
    const float early_speed = game.speed;
    game_update(&game, 100u);
    assert(game.speed > early_speed);

    game_init(&game, 7u);
    game_start(&game);
    for (size_t i = 0; i < DR_MAX_OBJECTS; ++i) game.objects[i].active = false;
    game.elapsed_s = 100.0f;
    game_update(&game, 1u);
    assert(game.speed == 28.0f);
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
    assert(counts.rects > 100u);
}

int main(void) {
    assert(smoke_test() == 0);
    test_tilt_deadzone_and_clamp();
    test_collision_transitions_to_game_over();
    test_grave_ring_buffer();
    test_restart_preserves_best_and_graves();
    test_speed_ramps_and_caps();
    test_renderer_uses_draw_callbacks();
    puts("host tests: PASS");
    return 0;
}
