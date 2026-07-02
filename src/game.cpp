#include "game.h"

#include "assets.h"
#include "camera.h"
#include "ui_helpers.h"
#include "sprites.h"
#include "title_screen_phase.h"
#include "world_phase.h"

static void GamePhaseSwitch(t_game *const game, const t_game_phase_id phase_id, const zcl::t_v2_i screen_size, const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const temp_arena) {
    zcl::ArenaRewind(game->phase_arena);

    game->phase_id = phase_id;

    switch (phase_id) {
        case ek_game_phase_id_title_screen: {
            game->phase_data = TitleScreenPhaseInit(game->assets, screen_size, game->phase_arena);
            break;
        }

        case ek_game_phase_id_world: {
            game->phase_data = WorldPhaseInit(gfx_ticket, screen_size, game->camera, game->phase_arena, temp_arena);
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }
}

static zcl::t_array_mut<t_cloud_layer *> CreateCloudLayers(t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_rng *const rng, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPushArray<t_cloud_layer *>(arena, 2);
    result[0] = CloudLayerCreate({screen_size.x / 256, screen_size.y / 180}, 0.85f, 0.05f, camera, screen_size, rng, arena);
    result[1] = CloudLayerCreate({screen_size.x / 256, screen_size.y / 180}, 0.85f, 0.1f, camera, screen_size, rng, arena);

    return result;
}

void GameInit(const zgl::t_game_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::WindowSetTitle(zf_context.platform_ticket, ZCL_STR_LITERAL("Terraria"), zf_context.temp_arena);
    zgl::CursorSetVisible(zf_context.platform_ticket, false);

    game->assets = AssetsCreate(zf_context.gfx_ticket, zf_context.rng, zf_context.perm_arena, zf_context.temp_arena);

    game->camera = CameraCreate(2.0f, 0.3f, zf_context.perm_arena);

    game->phase_arena = zcl::ArenaCreateBlockBased();

    GamePhaseSwitch(game, ek_game_phase_id_title_screen, zf_context.screen_size, zf_context.gfx_ticket, zf_context.temp_arena);

    const auto cloud_layer_arena_mem = zcl::ArenaPushArray<zcl::t_u8>(zf_context.perm_arena, zcl::KilobytesToBytes(4));
    game->cloud_layer_arena = zcl::ArenaCreateWrapping(cloud_layer_arena_mem);
    game->cloud_layers = CreateCloudLayers(game->camera, zf_context.screen_size, zf_context.rng, game->cloud_layer_arena);
}

void GameDeinit(const zgl::t_game_deinit_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
    zcl::ArenaDestroy(game->phase_arena);
}

void GameTick(const zgl::t_game_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            const auto result = TitleScreenPhaseTick(static_cast<t_title_screen_phase *>(game->phase_data), game->assets, zf_context.input_state, zf_context.screen_size, zf_context.temp_arena);

            switch (result) {
                case ek_title_screen_phase_tick_result_id_normal: {
                    break;
                }

                case ek_title_screen_phase_tick_result_id_go_to_world: {
                    GamePhaseSwitch(game, ek_game_phase_id_world, zf_context.screen_size, zf_context.gfx_ticket, zf_context.temp_arena);
                    break;
                }

                case ek_title_screen_phase_tick_result_id_exit_game: {
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
            const auto result = WorldPhaseTick(static_cast<t_world_phase *>(game->phase_data), game->assets, game->camera, zf_context.input_state, zf_context.screen_size, zf_context.gfx_ticket, zf_context.temp_arena);

            switch (result) {
                case ek_world_phase_tick_result_id_normal: {
                    break;
                }

                case ek_world_phase_tick_result_id_go_to_title_screen: {
                    GamePhaseSwitch(game, ek_game_phase_id_title_screen, zf_context.screen_size, zf_context.gfx_ticket, zf_context.temp_arena);
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

    // Update the sky.
    for (zcl::t_i32 i = 0; i < game->cloud_layers.len; i++) {
        CloudLayerUpdate(game->cloud_layers[i], zf_context.gfx_ticket, game->camera, zf_context.screen_size, game->assets);
    }
}

void GameRender(const zgl::t_game_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    // Render the sky background.
    constexpr zcl::t_color_rgba32f k_sky_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);
    zgl::RendererPassBegin(zf_context.rendering_context, zf_context.rendering_context.screen_size, zcl::MatrixCreateIdentity(), true, k_sky_color);
    zgl::RendererPassEnd(zf_context.rendering_context);

    for (zcl::t_i32 i = 0; i < game->cloud_layers.len; i++) {
        CloudLayerRender(game->cloud_layers[i], zf_context.rendering_context, game->assets, game->camera);
    }

    // Do pre-UI rendering.
    switch (game->phase_id) {
        case ek_game_phase_id_world: {
            WorldPhaseRender(static_cast<t_world_phase *>(game->phase_data), zf_context.rendering_context, game->assets, game->camera, zf_context.temp_arena);
            break;
        }

        case ek_game_phase_id_title_screen: {
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }

    // ----------------------------------------
    // UI

    zgl::RendererPassBegin(zf_context.rendering_context, zf_context.rendering_context.screen_size);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            TitleScreenPhaseRenderUI(static_cast<t_title_screen_phase *>(game->phase_data), zf_context.rendering_context, game->assets, zf_context.temp_arena);
            break;
        }

        case ek_game_phase_id_world: {
            WorldPhaseRenderUI(static_cast<t_world_phase *>(game->phase_data), zf_context.rendering_context, game->assets, game->camera, zf_context.input_state, zf_context.temp_arena);
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }

    zcl::t_static_array<zcl::t_u8, 32> fps_str_bytes = {};
    auto fps_str_bytes_stream = zcl::ByteStreamCreate(fps_str_bytes, zcl::ek_stream_mode_write);
    zcl::PrintFormat(zcl::ByteStreamGetView(&fps_str_bytes_stream), ZCL_STR_LITERAL("FPS: %"), zf_context.fps);

    RenderStrWithOutline(zf_context.rendering_context, {zcl::ByteStreamGetWritten(&fps_str_bytes_stream)}, *FontGet(game->assets, ek_font_id_roboto_40), {32.0f, static_cast<zcl::t_f32>(zf_context.rendering_context.screen_size.y - 32.0f)}, zcl::k_color_white, zf_context.temp_arena, zcl::k_origin_bottom_left);

    SpriteRender(ek_sprite_id_cursor, zf_context.rendering_context, game->assets, zgl::CursorGetPos(zf_context.input_state), zcl::k_origin_center);

    zgl::RendererPassEnd(zf_context.rendering_context);

    // ------------------------------
}

void GameProcessScreenResize(const zgl::t_game_screen_resize_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            TitleScreenPhaseProcessScreenResize(static_cast<t_title_screen_phase *>(game->phase_data), zf_context.screen_size, game->assets);
            break;
        }

        case ek_game_phase_id_world: {
            WorldPhaseProcessScreenResize(static_cast<t_world_phase *>(game->phase_data), zf_context.screen_size, game->camera);
            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }

    zcl::ArenaRewind(game->cloud_layer_arena);
    game->cloud_layers = CreateCloudLayers(game->camera, zf_context.screen_size, zf_context.rng, game->cloud_layer_arena);
}
