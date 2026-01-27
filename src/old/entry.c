#include <stdlib.h>
#include "game.h"

t_s32 main() {
    const s_game_info game_info = {
        .window_init_size = {1280, 720},
        .window_title = ARRAY_FROM_STATIC(GAME_TITLE),
        .window_flags = ek_window_flags_hide_cursor | ek_window_flags_resizable,

        .dev_mem_size = sizeof(s_game),
        .dev_mem_alignment = ALIGN_OF(s_game),

        .targ_ticks_per_sec = 60,

        .init_func = InitGame,
        .tick_func = GameTick,
        .render_func = RenderGame,
        .clean_func = CleanGame
    };

    return RunGame(&game_info) ? EXIT_SUCCESS : EXIT_FAILURE;
}
