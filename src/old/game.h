#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

// ==================================================

enum t_game_phase_id : zcl::t_i32 {
    ek_game_phase_id_none,
    ek_game_phase_id_title_screen,
    ek_game_phase_id_world
};

struct t_game {
    t_assets *assets;

    zcl::t_arena *phase_arena; // Lifetime is per-phase.
    t_game_phase_id phase_id;
    void *phase_data;
};

void GameInit(const zgl::t_game_init_func_context &zf_context);
void GameDeinit(const zgl::t_game_deinit_func_context &zf_context);
void GameTick(const zgl::t_game_tick_func_context &zf_context);
void GameRender(const zgl::t_game_render_func_context &zf_context);
void GameProcessScreenResize(const zgl::t_game_screen_resize_func_context &zf_context);
