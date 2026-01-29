#pragma once


// ============================================================
// @section: External Forward Declarations
// ============================================================

struct t_assets;

// ============================================================


struct t_title_screen;

t_title_screen *TitleScreenInit(const t_assets *const assets, const zgl::t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const arena);

enum t_title_screen_tick_result_id : zcl::t_i32 {
    ek_title_screen_tick_result_id_normal,
    ek_title_screen_tick_result_id_go_to_world,
    ek_title_screen_tick_result_id_exit_game
};

[[nodiscard]] t_title_screen_tick_result_id TitleScreenTick(t_title_screen *const ts, const t_assets *const assets, const zgl::t_input_state *const input_state, const zgl::t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const temp_arena);

void TitleScreenRenderUI(const t_title_screen *const ts, const zgl::t_rendering_context rendering_context, const t_assets *const assets, zcl::t_arena *const temp_arena);

void TitleScreenProcessBackbufferResize(t_title_screen *const ts, const zcl::t_v2_i backbuffer_size, const t_assets *const assets);
