#pragma once


// ============================================================
// @section: External Forward Declarations
// ============================================================

struct t_assets;
struct t_world;

// ============================================================


struct t_game {
    t_assets *assets;
    t_world *world;
};

void GameInit(const zgl::t_game_init_func_context &zf_context);
void GameDeinit(const zgl::t_game_deinit_func_context &zf_context);
void GameTick(const zgl::t_game_tick_func_context &zf_context);
void GameRender(const zgl::t_game_render_func_context &zf_context);
