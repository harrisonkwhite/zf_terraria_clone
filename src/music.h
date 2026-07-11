#pragma once

#include "assets.h"

// ============================================================
// @section: External Forward Declarations

struct t_options;

// ==================================================

struct t_music_manager;

t_music_manager *MusicManagerCreate(const zcl::t_f32 fade_time_secs, const zcl::t_i32 fade_cnt, const t_options *const options, const t_assets *const assets, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const arena);

void MusicManagerSetCurrent(t_music_manager *const manager, const t_music_type_id type_id);

void MusicManagerUpdate(t_music_manager *const manager);
