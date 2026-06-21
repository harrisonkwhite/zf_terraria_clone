#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

// ==================================================

struct t_world_phase;

t_world_phase *WorldPhaseInit(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

enum t_world_phase_tick_result_id : zcl::t_i32 {
    ek_world_phase_tick_result_id_normal,
    ek_world_phase_tick_result_id_go_to_title_screen
};

[[nodiscard]] t_world_phase_tick_result_id WorldPhaseTick(t_world_phase *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);

void WorldPhaseRender(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena);

void WorldPhaseRenderUI(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);
