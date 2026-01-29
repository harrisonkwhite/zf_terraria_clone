#include "game.h"

int main() {
    zgl::t_game_config config = zgl::GameConfigCreate(GameInit, GameDeinit, GameTick, GameRender);
    config.backbuffer_resize_func = GameProcessBackbufferResize;
    config.user_mem_size = ZCL_SIZE_OF(t_game);
    config.user_mem_alignment = ZCL_ALIGN_OF(t_game);

    zgl::GameRun(config);
}
