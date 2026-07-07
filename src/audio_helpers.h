#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_options;

// ==================================================

zcl::t_f32 CalcSoundVolumeWithOptions(const t_options *const opts, const zcl::t_f32 vol = 1.0f);

zcl::t_f32 CalcMusicVolumeWithOptions(const t_options *const opts, const zcl::t_f32 vol = 1.0f);

// Wraps around the standard ZGL SoundFireAndForget but alters the volume based on the provided game options (e.g. affected by master volume).
inline zcl::t_b8 SoundFireAndForgetWithOptions(const zgl::t_audio_ticket_mut audio_ticket, const zgl::t_sound_type *const type, const t_options *const opts, const zcl::t_f32 vol = 1.0f, const zcl::t_f32 pan = 0.0f, const zcl::t_f32 pitch = 1.0f) {
    return zgl::SoundFireAndForget(audio_ticket, type, CalcSoundVolumeWithOptions(opts, vol), pan, pitch);
}
