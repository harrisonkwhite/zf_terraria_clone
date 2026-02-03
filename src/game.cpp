#include "game.h"

#include "assets.h"
#include "sprites.h"
#include "title_screen.h"
#include "world_public.h"

static void GamePhaseSwitch(t_game *const game, const t_game_phase_id phase_id, const t_assets *const assets, const zcl::t_v2_i screen_size, const zgl::t_gfx_ticket_mut gfx_ticket) {
    zcl::ArenaRewind(game->phase_arena);

    game->phase_id = phase_id;

    switch (phase_id) {
        case ek_game_phase_id_title_screen: {
            game->phase_data = TitleScreenInit(assets, screen_size, game->phase_arena);
            break;
        }

        case ek_game_phase_id_world: {
            game->phase_data = WorldCreate(gfx_ticket, game->phase_arena);
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }
}

void GameInit(const zgl::t_game_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::WindowSetTitle(zf_context.platform_ticket, ZCL_STR_LITERAL("Terraria"), zf_context.temp_arena);
    zgl::CursorSetVisible(zf_context.platform_ticket, false);

    game->assets = AssetsCreate(zf_context.gfx_ticket, zf_context.perm_arena, zf_context.temp_arena);

    game->phase_arena = zcl::ArenaCreateBlockBased();

    GamePhaseSwitch(game, ek_game_phase_id_title_screen, game->assets, zf_context.screen_size, zf_context.gfx_ticket);
}

void GameDeinit(const zgl::t_game_deinit_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
    zcl::ArenaDestroy(game->phase_arena);
}

void GameTick(const zgl::t_game_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            const auto result = TitleScreenTick(static_cast<t_title_screen *>(game->phase_data), game->assets, zf_context.input_state, zf_context.screen_size, zf_context.temp_arena);

            switch (result) {
                case ek_title_screen_tick_result_id_normal: {
                    break;
                }

                case ek_title_screen_tick_result_id_go_to_world: {
                    GamePhaseSwitch(game, ek_game_phase_id_world, game->assets, zf_context.screen_size, zf_context.gfx_ticket);
                    break;
                }

                case ek_title_screen_tick_result_id_exit_game: {
                    zgl::WindowRequestClose(zf_context.platform_ticket);
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }

            break;
        }

        case ek_game_phase_id_world: {
            const auto result = WorldTick(static_cast<t_world *>(game->phase_data), game->assets, zf_context.input_state, zf_context.screen_size, zf_context.temp_arena);

            switch (result) {
                case ek_world_tick_result_id_normal: {
                    break;
                }

                case ek_world_tick_result_id_go_to_title_screen: {
                    GamePhaseSwitch(game, ek_game_phase_id_title_screen, game->assets, zf_context.screen_size, zf_context.gfx_ticket);
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }

            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }
}

void GameRender(const zgl::t_game_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    // Do a dummy pass just to make sure everything gets cleared.
    zgl::RendererPassBegin(zf_context.rendering_context, zf_context.rendering_context.screen_size, zcl::MatrixCreateIdentity(), true);
    zgl::RendererPassEnd(zf_context.rendering_context);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            break;
        }

        case ek_game_phase_id_world: {
            WorldRender(static_cast<t_world *>(game->phase_data), zf_context.rendering_context, game->assets, zf_context.input_state);
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }

    zgl::RendererPassBegin(zf_context.rendering_context, zf_context.rendering_context.screen_size);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            TitleScreenRenderUI(static_cast<t_title_screen *>(game->phase_data), zf_context.rendering_context, game->assets, zf_context.temp_arena);
            break;
        }

        case ek_game_phase_id_world: {
            UIRender(static_cast<t_world *>(game->phase_data), zf_context.rendering_context, game->assets, zf_context.input_state, zf_context.temp_arena);
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }

    SpriteRender(ek_sprite_id_cursor, zf_context.rendering_context, game->assets, zgl::CursorGetPos(zf_context.input_state), zcl::k_origin_center);

    zgl::RendererPassEnd(zf_context.rendering_context);
}

void GameProcessScreenResize(const zgl::t_game_screen_resize_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            TitleScreenProcessScreenResize(static_cast<t_title_screen *>(game->phase_data), zf_context.screen_size, game->assets);
            break;
        }

        case ek_game_phase_id_world: {
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }
}
