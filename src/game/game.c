#include "game.h"
#include <string.h>

static uint32_t next_random(dr_game_t *g) {
    uint32_t x = g->rng ? g->rng : 0x9e3779b9u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    g->rng = x;
    return x;
}

static void spawn_object(dr_game_t *g) {
    for (size_t i = 0; i < DR_MAX_OBJECTS; ++i) {
        dr_object_t *o = &g->objects[i];
        if (!o->active) {
            o->active = true;
            o->x = ((float)(next_random(g) % 1801u) / 900.0f) - 1.0f;
            o->z = 65.0f + (float)(next_random(g) % 700u) / 10.0f;
            o->type = DR_OBJ_TREE;
            return;
        }
    }
}

void game_init(dr_game_t *g, uint32_t seed) {
    if (!g) return;
    memset(g, 0, sizeof(*g));
    g->state = DR_STATE_READY;
    g->speed = 14.0f;
    g->rng = seed ? seed : 0x13579bdu;
    for (int i = 0; i < 7; ++i) {
        spawn_object(g);
        for (size_t j = 0; j < DR_MAX_OBJECTS; ++j) {
            if (g->objects[j].active && g->objects[j].z > 65.0f)
                g->objects[j].z = 12.0f + (float)i * 10.0f;
        }
    }
}

void game_start(dr_game_t *g) {
    if (g && g->state != DR_STATE_RUNNING) {
        g->state = DR_STATE_RUNNING;
        g->player_x = 0.0f;
        g->distance = 0.0f;
        g->elapsed_s = 0.0f;
        g->score = 0;
    }
}

void game_set_input(dr_game_t *g, float axis) {
    if (!g) return;
    if (axis < -1.0f) axis = -1.0f;
    if (axis > 1.0f) axis = 1.0f;
    g->input_axis = axis;
}

void game_update(dr_game_t *g, uint32_t dt_ms) {
    if (!g || g->state != DR_STATE_RUNNING) return;
    if (dt_ms > 100) dt_ms = 100;
    const float dt = (float)dt_ms / 1000.0f;
    g->elapsed_s += dt;
    g->distance += g->speed * dt;
    g->score = (uint32_t)g->distance;
    g->player_x += g->input_axis * 1.65f * dt;
    if (g->player_x < -0.92f) g->player_x = -0.92f;
    if (g->player_x >  0.92f) g->player_x =  0.92f;
    for (size_t i = 0; i < DR_MAX_OBJECTS; ++i) {
        dr_object_t *o = &g->objects[i];
        if (!o->active) continue;
        o->z -= g->speed * dt;
        if (o->z < 0.8f) {
            if (o->z > -1.5f && o->type == DR_OBJ_TREE &&
                (o->x - g->player_x < 0.14f) && (g->player_x - o->x < 0.14f)) {
                g->state = DR_STATE_GAME_OVER;
                return;
            }
            o->active = false;
        }
    }
    if (next_random(g) % 100u < (uint32_t)(dt * 24.0f)) spawn_object(g);
}

void game_restart(dr_game_t *g) {
    if (!g) return;
    const uint32_t seed = g->rng ^ (uint32_t)(g->distance * 97.0f) ^ 0xa511e9b3u;
    game_init(g, seed);
    game_start(g);
}

void game_add_grave(dr_game_t *g, float x, float z) {
    if (!g) return;
    if (x < -1.0f) x = -1.0f;
    if (x > 1.0f) x = 1.0f;
    if (z < 0.0f) z = 0.0f;
    size_t slot;
    if (g->grave_count < DR_MAX_GRAVES) {
        slot = g->grave_count++;
    } else {
        slot = g->grave_cursor++ % DR_MAX_GRAVES;
    }
    g->graves[slot].x = x;
    g->graves[slot].z = z;
}

void game_set_tilt_input(dr_game_t *g, float tilt, float deadzone, float gain) {
    if (deadzone < 0.0f) deadzone = 0.0f;
    if (gain < 0.0f) gain = 0.0f;
    if (tilt > -deadzone && tilt < deadzone) tilt = 0.0f;
    else if (tilt > 0.0f) tilt -= deadzone;
    else tilt += deadzone;
    game_set_input(g, tilt * gain);
}
