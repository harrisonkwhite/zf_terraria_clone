#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

// ==================================================

struct t_phase_world;

t_phase_world *PhaseWorldInit(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena);

enum t_phase_world_tick_result_id : zcl::t_i32 {
    ek_phase_world_tick_result_id_normal,
    ek_phase_world_tick_result_id_go_to_title_screen
};

[[nodiscard]] t_phase_world_tick_result_id PhaseWorldTick(t_phase_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);

void PhaseWorldRender(const t_phase_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state);

void PhaseWorldRenderUI(const t_phase_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);
