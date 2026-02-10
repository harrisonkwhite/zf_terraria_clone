#include "game.h"

#include "assets.h"
#include "sprites.h"

void GameInit(const zgl::t_game_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::WindowSetTitle(zf_context.platform_ticket, ZCL_STR_LITERAL("Terraria"), zf_context.temp_arena);
    zgl::CursorSetVisible(zf_context.platform_ticket, false);

    game->assets = AssetsCreate(zf_context.gfx_ticket, zf_context.perm_arena, zf_context.temp_arena);

    game->phase_arena = zcl::ArenaCreateBlockBased();
}

void GameDeinit(const zgl::t_game_deinit_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
    zcl::ArenaDestroy(game->phase_arena);
}

void GameTick(const zgl::t_game_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
}

void GameRender(const zgl::t_game_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    // Do a dummy pass just to make sure everything gets cleared.
    zgl::RendererPassBegin(zf_context.rendering_context, zf_context.rendering_context.screen_size, zcl::MatrixCreateIdentity(), true);
    zgl::RendererPassEnd(zf_context.rendering_context);

    // ----------------------------------------
    // UI

    zgl::RendererPassBegin(zf_context.rendering_context, zf_context.rendering_context.screen_size);

    zcl::t_static_array<zcl::t_u8, 32> fps_str_bytes = {};
    auto fps_str_bytes_stream = zcl::ByteStreamCreate(fps_str_bytes, zcl::ek_stream_mode_write);
    zcl::PrintFormat(zcl::ByteStreamGetView(&fps_str_bytes_stream), ZCL_STR_LITERAL("FPS: %"), zf_context.fps);

    zgl::RendererSubmitStr(zf_context.rendering_context, {zcl::ByteStreamGetWritten(&fps_str_bytes_stream)}, *GetFont(game->assets, ek_font_id_eb_garamond_48), {32.0f, static_cast<zcl::t_f32>(zf_context.rendering_context.screen_size.y - 32.0f)}, zcl::k_color_white, zf_context.temp_arena, zcl::k_origin_bottom_left);

    SpriteRender(ek_sprite_id_cursor, zf_context.rendering_context, game->assets, zgl::CursorGetPos(zf_context.input_state), zcl::k_origin_center);

    zgl::RendererPassEnd(zf_context.rendering_context);

    // ------------------------------
}

void GameProcessScreenResize(const zgl::t_game_screen_resize_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);
}
