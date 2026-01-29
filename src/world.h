#pragma once


// ============================================================
// @section: External Forward Declarations
// ============================================================

struct t_assets;

// ============================================================


struct t_world;

t_world *WorldCreate(zcl::t_arena *const arena);
void WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i window_framebuffer_size, zcl::t_arena *const temp_arena);
void WorldRender(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state);
void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);
