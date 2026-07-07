#include "game.h"

#include "options.h"
#include "assets.h"
#include "sky.h"
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
            game->phase_data = TitleScreenPhaseInit(screen_size, game->phase_arena);
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

static t_options *GameOptionsCreate(zcl::t_arena *const arena) {
    static const zcl::t_static_array<zcl::t_str_rdonly, 11> g_perc_names = {{
        ZCL_STR_LITERAL("0%"),
        ZCL_STR_LITERAL("10%"),
        ZCL_STR_LITERAL("20%"),
        ZCL_STR_LITERAL("30%"),
        ZCL_STR_LITERAL("40%"),
        ZCL_STR_LITERAL("50%"),
        ZCL_STR_LITERAL("60%"),
        ZCL_STR_LITERAL("70%"),
        ZCL_STR_LITERAL("80%"),
        ZCL_STR_LITERAL("90%"),
        ZCL_STR_LITERAL("100%"),
    }};

    static const zcl::t_static_array<zcl::t_f32, 11> g_perc_f32s = {{
        0.0f,
        0.1f,
        0.2f,
        0.3f,
        0.4f,
        0.5f,
        0.6f,
        0.7f,
        0.8f,
        0.9f,
        1.0f,
    }};

    static const zcl::t_static_array<zcl::t_str_rdonly, 2> g_toggle_names = {{
        ZCL_STR_LITERAL("Disabled"),
        ZCL_STR_LITERAL("Enabled"),
    }};

    static const zcl::t_static_array<zcl::t_b8, 2> g_toggle_b8s = {{
        false,
        true,
    }};

    const auto result = OptionsCreate(arena);

    for (zcl::t_i32 i = 0; i < ekm_option_id_cnt; i++) {
        switch (static_cast<t_option_id>(i)) {
            case ek_option_id_master_volume:
            case ek_option_id_sound_volume:
            case ek_option_id_music_volume: {
                OptionRegisterValueSeqF32(result, static_cast<t_option_id>(i), g_perc_names, g_perc_f32s);
                OptionSetValueIndex(result, static_cast<t_option_id>(i), g_perc_names.k_len - 1);
                break;
            }

            case ek_option_id_fullscreen: {
                OptionRegisterValueSeqB8(result, static_cast<t_option_id>(i), g_toggle_names, g_toggle_b8s);
                break;
            }

            case ekm_option_id_cnt: {
                ZCL_UNREACHABLE();
            }
        }
    }

    return result;
}

static t_sky *GameSkyCreate(const zcl::t_v2_i screen_size, t_camera *const camera, zcl::t_rng *const rng, zcl::t_arena *const arena) {
    const zcl::t_static_array<t_sky_layer_info, 2> layer_infos = {{
        {.parallax = 0.05f, .cloud_grid_dims = {screen_size.x / 256, screen_size.y / 180}, .cloud_chance = 0.85f},
        {.parallax = 0.1f, .cloud_grid_dims = {screen_size.x / 256, screen_size.y / 180}, .cloud_chance = 0.85f},
    }};

    return SkyCreate(layer_infos, camera, screen_size, rng, arena);
}

void GameInit(const zgl::t_game_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::WindowSetTitle(zf_context.platform_ticket, ZCL_STR_LITERAL("Terraria"), zf_context.temp_arena);
    zgl::CursorSetVisible(zf_context.platform_ticket, false);

    game->options = GameOptionsCreate(zf_context.perm_arena);

    game->assets = AssetsCreate(zf_context.gfx_ticket, zf_context.audio_ticket, zf_context.rng, zf_context.perm_arena, zf_context.temp_arena);

    game->camera = CameraCreate(2.0f, 0.3f, zf_context.perm_arena);

    game->phase_arena = zcl::ArenaCreateBlockBased();

    GamePhaseSwitch(game, ek_game_phase_id_title_screen, zf_context.screen_size, zf_context.gfx_ticket, zf_context.temp_arena);

    const auto sky_arena_mem = zcl::ArenaPushArray<zcl::t_u8>(zf_context.perm_arena, zcl::KilobytesToBytes(4));
    game->sky_arena = zcl::ArenaCreateWrapping(sky_arena_mem);

    game->sky = GameSkyCreate(zf_context.screen_size, game->camera, zf_context.rng, game->sky_arena);
}

void GameDeinit(const zgl::t_game_deinit_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
    AssetsDestroy(game->assets, zf_context.gfx_ticket, zf_context.audio_ticket, zf_context.temp_arena);
    zcl::ArenaDestroy(game->phase_arena);
}

void GameTick(const zgl::t_game_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->phase_id) {
        case ek_game_phase_id_title_screen: {
            const auto result = TitleScreenPhaseTick(static_cast<t_title_screen_phase *>(game->phase_data), game->options, game->assets, zf_context.input_state, zf_context.screen_size, zf_context.audio_ticket, zf_context.temp_arena);

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
            const auto result = WorldPhaseTick(static_cast<t_world_phase *>(game->phase_data), game->options, game->assets, game->camera, zf_context.input_state, zf_context.screen_size, zf_context.gfx_ticket, zf_context.audio_ticket, zf_context.temp_arena);

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

    SkyUpdate(game->sky, zf_context.gfx_ticket, game->camera, zf_context.screen_size, game->assets);

    zgl::WindowSetFullscreen(zf_context.platform_ticket, OptionGetValueB8(game->options, ek_option_id_fullscreen));
}

void GameRender(const zgl::t_game_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    SkyRender(game->sky, zf_context.rendering_context, game->assets, game->camera);

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
            TitleScreenPhaseRenderUI(static_cast<t_title_screen_phase *>(game->phase_data), zf_context.rendering_context, game->options, game->assets, zf_context.temp_arena);
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
            TitleScreenPhaseProcessScreenResize(static_cast<t_title_screen_phase *>(game->phase_data), zf_context.screen_size);
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

    zcl::ArenaRewind(game->sky_arena);
    game->sky = GameSkyCreate(zf_context.screen_size, game->camera, zf_context.rng, game->sky_arena);
}
