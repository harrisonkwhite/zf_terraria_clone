#include "game.h"

#include "assets.h"
#include "title_screen.h"
#include "world.h"

static void GamePhaseUpdate(t_game *const game, const t_game_phase_id phase_id) {
    zcl::ArenaRewind(&game->phase_arena);

    game->phase_id = phase_id;

    switch (phase_id) {
    case ek_game_phase_id_title_screen:
        game->phase_data = TitleScreenInit(&game->phase_arena);
        break;

    case ek_game_phase_id_world:
        game->phase_data = WorldCreate(&game->phase_arena);
        break;

    default:
        ZCL_UNREACHABLE();
    }
}

void GameInit(const zgl::t_game_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    game->assets = AssetsCreate(zf_context.gfx_ticket, zf_context.perm_arena, zf_context.temp_arena);

    game->phase_arena = zcl::ArenaCreateBlockBased();

    GamePhaseUpdate(game, ek_game_phase_id_title_screen);
}

void GameDeinit(const zgl::t_game_deinit_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
}

void GameTick(const zgl::t_game_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
}

void GameRender(const zgl::t_game_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::RendererPassBegin(zf_context.rendering_context, zgl::BackbufferGetSize(zf_context.rendering_context.gfx_ticket));
    zgl::RendererPassEnd(zf_context.rendering_context);
}
