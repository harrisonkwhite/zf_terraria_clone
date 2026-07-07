#include "audio_helpers.h"

#include "options.h"

zcl::t_f32 CalcSoundVolumeWithOptions(const t_options *const opts, const zcl::t_f32 vol) {
    ZCL_ASSERT(zcl::RangeValueCheckWithin(zgl::k_sound_volume_range, vol));
    return OptionGetValueF32(opts, ek_option_id_master_volume) * OptionGetValueF32(opts, ek_option_id_sound_volume);
}

zcl::t_f32 CalcMusicVolumeWithOptions(const t_options *const opts, const zcl::t_f32 vol) {
    ZCL_ASSERT(zcl::RangeValueCheckWithin(zgl::k_sound_volume_range, vol));
    return OptionGetValueF32(opts, ek_option_id_master_volume) * OptionGetValueF32(opts, ek_option_id_music_volume);
}
