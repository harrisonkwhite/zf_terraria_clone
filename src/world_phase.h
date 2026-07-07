#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

struct t_options;

// ==================================================

struct t_world_phase;

t_world_phase *WorldPhaseInit(const zgl::t_gfx_ticket_mut gfx_ticket, const zcl::t_v2_i screen_size, t_camera *const camera, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

enum t_world_phase_tick_result_id : zcl::t_i32 {
    ek_world_phase_tick_result_id_normal,
    ek_world_phase_tick_result_id_go_to_title_screen
};

[[nodiscard]] t_world_phase_tick_result_id WorldPhaseTick(t_world_phase *const world, const t_options *const options, const t_assets *const assets, t_camera *const camera, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, const zgl::t_gfx_ticket_rdonly gfx_ticket, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena);

void WorldPhaseRender(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, t_camera *const camera, zcl::t_arena *const temp_arena);

void WorldPhaseRenderUI(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, t_camera *const camera, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);

void WorldPhaseProcessScreenResize(t_world_phase *const world, const zcl::t_v2_i screen_size, t_camera *const camera);
