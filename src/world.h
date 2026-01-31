#pragma once


// ============================================================
// @section: External Forward Declarations
// ============================================================

struct t_assets;

// ============================================================


struct t_world;

t_world *WorldCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena);
void WorldDestroy(t_world *const world, const zgl::t_gfx_ticket_mut gfx_ticket);

enum t_world_tick_result_id : zcl::t_i32 {
    ek_world_tick_result_id_normal,
    ek_world_tick_result_id_go_to_title_screen,
};

[[nodiscard]] t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i window_framebuffer_size, zcl::t_arena *const temp_arena);

void WorldRender(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state);

void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);
