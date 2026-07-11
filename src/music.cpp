#include "music.h"

#include "options.h"
#include "audio_helpers.h"

struct t_music_manager {
    zcl::t_f32 fade_time_secs;

    zcl::t_b8 snd_started;
    zgl::t_sound_id snd_id;

    zcl::t_array_mut<zgl::t_sound_id> fade_snd_ids;
    zcl::t_i32 fade_snd_id_begin_index;
    zcl::t_i32 fade_snd_id_cnt;

    const t_options *options;
    const t_assets *assets;
    zgl::t_audio_ticket_mut audio_ticket;
};

t_music_manager *MusicManagerCreate(const zcl::t_f32 fade_time_secs, const zcl::t_i32 fade_cnt, const t_options *const options, const t_assets *const assets, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const arena) {
    ZCL_ASSERT(fade_time_secs >= 0.0f);
    ZCL_ASSERT(fade_cnt > 0);

    const auto result = zcl::ArenaPush<t_music_manager>(arena);

    result->fade_time_secs = fade_time_secs;

    result->fade_snd_ids = zcl::ArenaPushArray<zgl::t_sound_id>(arena, fade_cnt);

    result->options = options;
    result->assets = assets;
    result->audio_ticket = audio_ticket;

    return result;
}

void MusicManagerSet(t_music_manager *const manager, const t_music_type_id type_id) {
    if (manager->snd_started) {
        if (manager->fade_snd_id_cnt == manager->fade_snd_ids.len) {
            // All slots full, so evict the oldest music sound instance.
            zgl::SoundDestroy(manager->audio_ticket, manager->fade_snd_ids[manager->fade_snd_id_begin_index]);

            manager->fade_snd_id_begin_index = (manager->fade_snd_id_begin_index + 1) % manager->fade_snd_ids.len;
            manager->fade_snd_id_cnt--;
        }

        // Fade the current playing music sound instance, and add to the back of the fade queue.
        zgl::SoundSetVolumeFade(manager->audio_ticket, manager->snd_id, 1.0f, 0.0f, manager->fade_time_secs);

        manager->fade_snd_ids[(manager->fade_snd_id_begin_index + manager->fade_snd_id_cnt) % manager->fade_snd_ids.len] = manager->snd_id;
        manager->fade_snd_id_cnt++;
    }

    if (!zgl::SoundCreate(manager->audio_ticket, MusicTypeGet(manager->assets, type_id), &manager->snd_id)) {
        ZCL_FATAL();
    }

    zgl::SoundSetVolume(manager->audio_ticket, manager->snd_id, CalcMusicVolumeWithOptions(manager->options));
    zgl::SoundSetVolumeFade(manager->audio_ticket, manager->snd_id, 0.0f, 1.0f, manager->fade_time_secs);
    zgl::SoundSetLooping(manager->audio_ticket, manager->snd_id, true);

    zgl::SoundStart(manager->audio_ticket, manager->snd_id);

    manager->snd_started = true;
}

void MusicManagerUpdate(t_music_manager *const manager) {
    if (manager->snd_started) {
        const auto music_vol = CalcMusicVolumeWithOptions(manager->options);

        zgl::SoundSetVolume(manager->audio_ticket, manager->snd_id, music_vol);

        if (manager->fade_snd_id_cnt > 0) {
            // If the fade of the music sound instance at the front of the queue is complete, destroy the sound instance.
            const auto snd_id = manager->fade_snd_ids[manager->fade_snd_id_begin_index];

            if (zgl::SoundGetFadeVolume(manager->audio_ticket, snd_id) == 0.0f) {
                zgl::SoundDestroy(manager->audio_ticket, snd_id);

                manager->fade_snd_id_begin_index = (manager->fade_snd_id_begin_index + 1) % manager->fade_snd_ids.len;
                manager->fade_snd_id_cnt--;
            }
        }

        // Ensure volumes of fading music sound instance still match the option settings.
        for (zcl::t_i32 i = 0; i < manager->fade_snd_id_cnt; i++) {
            const zcl::t_i32 snd_id_index = (manager->fade_snd_id_begin_index + i) % manager->fade_snd_ids.len;
            zgl::SoundSetVolume(manager->audio_ticket, manager->fade_snd_ids[snd_id_index], music_vol);
        }
    }
}
