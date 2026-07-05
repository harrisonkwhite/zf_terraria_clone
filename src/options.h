#pragma once

enum t_option_type_id : zcl::t_i32 {
    ek_option_type_id_perc,
    ek_option_type_id_toggle
};

enum t_option_id : zcl::t_i32 {
    ek_option_id_master_volume,
    ek_option_id_sound_volume,
    ek_option_id_music_volume,
    ek_option_id_fullscreen,

    ekm_option_id_cnt
};

struct t_option_meta {
    zcl::t_str_rdonly name;
    t_option_type_id type_id;
};

inline const zcl::t_static_array<t_option_meta, ekm_option_id_cnt> g_option_metas = {{
    {
        .name = ZCL_STR_LITERAL("Master Volume"),
        .type_id = ek_option_type_id_perc,
    },
    {
        .name = ZCL_STR_LITERAL("Sound Volume"),
        .type_id = ek_option_type_id_perc,
    },
    {
        .name = ZCL_STR_LITERAL("Music Volume"),
        .type_id = ek_option_type_id_perc,
    },
    {
        .name = ZCL_STR_LITERAL("Fullscreen"),
        .type_id = ek_option_type_id_toggle,
    },
}};

struct t_options;

t_options *OptionsCreate(zcl::t_arena *const arena);

zcl::t_f32 OptionsGetPerc(const t_options *const opts, const t_option_id id);
void OptionsSetPerc(t_options *const opts, const t_option_id id, const zcl::t_f32 value);

zcl::t_b8 OptionsGetToggle(const t_options *const opts, const t_option_id id);
void OptionsSetToggle(t_options *const opts, const t_option_id id, const zcl::t_b8 value);
