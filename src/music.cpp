#include "music.h"

#include "options.h"
#include "audio_helpers.h"

struct t_music_manager {
    zcl::t_b8 snd_started;
    zgl::t_sound_id snd_id;

    const t_options *options;
    const t_assets *assets;
    zgl::t_audio_ticket_mut audio_ticket;
};

t_music_manager *MusicManagerCreate(const zcl::t_i32 fade_cnt, const t_options *const options, const t_assets *const assets, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const arena) {
    ZCL_ASSERT(fade_cnt > 0);

    const auto result = zcl::ArenaPush<t_music_manager>(arena);
    result->options = options;
    result->assets = assets;
    result->audio_ticket = audio_ticket;

    return result;
}

void MusicManagerSet(t_music_manager *const manager, const t_music_type_id type_id) {
    if (manager->snd_started) {
        zgl::SoundDestroy(manager->audio_ticket, manager->snd_id);
    }

    if (!zgl::SoundCreate(manager->audio_ticket, MusicTypeGet(manager->assets, type_id), &manager->snd_id)) {
        ZCL_FATAL();
    }

    zgl::SoundSetVolume(manager->audio_ticket, manager->snd_id, CalcMusicVolumeWithOptions(manager->options));
    zgl::SoundSetVolumeFade(manager->audio_ticket, manager->snd_id, 0.0f, 1.0f, 2.0f);
    zgl::SoundSetLooping(manager->audio_ticket, manager->snd_id, true);

    zgl::SoundStart(manager->audio_ticket, manager->snd_id);

    manager->snd_started = true;
}

void MusicManagerUpdate(t_music_manager *const manager) {
    if (manager->snd_started) {
        zgl::SoundSetVolume(manager->audio_ticket, manager->snd_id, CalcMusicVolumeWithOptions(manager->options));
    }
}
