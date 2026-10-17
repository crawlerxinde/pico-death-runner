#include "game.h"
#include <assert.h>
int main(void) {
    dr_game_t g;
    game_init(&g, 1234u);
    assert(g.state == DR_STATE_READY);
    game_start(&g);
    game_set_input(&g, 1.0f);
    game_update(&g, 100);
    assert(g.state == DR_STATE_RUNNING);
    assert(g.player_x > 0.0f && g.player_x <= 0.92f);
    return 0;
}
