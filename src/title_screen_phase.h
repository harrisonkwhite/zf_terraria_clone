#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

// ==================================================

struct t_title_screen_phase;

t_title_screen_phase *TitleScreenPhaseInit(const zcl::t_v2_i screen_size, zcl::t_arena *const arena);

enum t_title_screen_phase_tick_result_id : zcl::t_i32 {
    ek_title_screen_phase_tick_result_id_normal,
    ek_title_screen_phase_tick_result_id_go_to_world,
    ek_title_screen_phase_tick_result_id_exit_game
};

[[nodiscard]] t_title_screen_phase_tick_result_id TitleScreenPhaseTick(t_title_screen_phase *const ts, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena);

void TitleScreenPhaseRenderUI(const t_title_screen_phase *const ts, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena);

void TitleScreenPhaseProcessScreenResize(t_title_screen_phase *const ts, const zcl::t_v2_i screen_size);
