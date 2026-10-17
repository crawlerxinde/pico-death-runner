#ifndef DEAD_RUNNER_GAME_H
#define DEAD_RUNNER_GAME_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DR_SCREEN_W 640
#define DR_SCREEN_H 172
#define DR_MAX_OBJECTS 24
#define DR_MAX_GRAVES 32

typedef enum { DR_OBJ_TREE = 0, DR_OBJ_GRAVE = 1 } dr_object_type_t;
typedef enum { DR_STATE_READY = 0, DR_STATE_RUNNING, DR_STATE_GAME_OVER } dr_state_t;

typedef struct {
    float x;
    float z;
    dr_object_type_t type;
    bool active;
} dr_object_t;

typedef struct {
    float x;
    float z;
} dr_grave_t;

typedef struct {
    int16_t x, y, w, h;
    dr_object_type_t type;
    bool visible;
} dr_projected_t;

typedef struct {
    uint16_t *pixels;
    int width;
    int height;
    int stride_pixels;
} dr_framebuffer_t;

typedef struct {
    void (*clear)(void *ctx, uint16_t rgb565);
    void (*rect)(void *ctx, int x, int y, int w, int h, uint16_t rgb565);
    void (*line)(void *ctx, int x1, int y1, int x2, int y2, uint16_t rgb565);
    void *ctx;
    int width;
    int height;
} dr_draw_api_t;

typedef struct {
    dr_state_t state;
    float player_x;
    float speed;
    float distance;
    float elapsed_s;
    float input_axis;
    uint32_t rng;
    dr_object_t objects[DR_MAX_OBJECTS];
    dr_grave_t graves[DR_MAX_GRAVES];
    size_t grave_count;
    size_t grave_cursor;
    uint32_t score;
} dr_game_t;

void game_init(dr_game_t *g, uint32_t seed);
void game_start(dr_game_t *g);
void game_restart(dr_game_t *g);
void game_set_input(dr_game_t *g, float axis);
void game_add_grave(dr_game_t *g, float x, float z);
void game_set_tilt_input(dr_game_t *g, float tilt, float deadzone, float gain);
void game_update(dr_game_t *g, uint32_t dt_ms);
void game_render(const dr_game_t *g, const dr_draw_api_t *draw);

#endif
