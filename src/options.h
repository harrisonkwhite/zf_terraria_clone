#pragma once

enum t_option_id : zcl::t_i32 {
    ek_option_id_master_volume,
    ek_option_id_sound_volume,
    ek_option_id_music_volume,
    ek_option_id_fullscreen,

    ekm_option_id_cnt
};

inline const zcl::t_static_array<zcl::t_str_rdonly, ekm_option_id_cnt> g_option_names = {{
    ZCL_STR_LITERAL("Master Volume"),
    ZCL_STR_LITERAL("Sound Volume"),
    ZCL_STR_LITERAL("Music Volume"),
    ZCL_STR_LITERAL("Fullscreen"),
}};

struct t_options;

t_options *OptionsCreate(zcl::t_arena *const arena);

void OptionRegisterValueSetB8(t_options *const opts, const t_option_id opt_id, const zcl::t_array_rdonly<zcl::t_str_rdonly> set_names, const zcl::t_array_rdonly<zcl::t_b8> set_b8s);

void OptionRegisterValueSetF32(t_options *const opts, const t_option_id opt_id, const zcl::t_array_rdonly<zcl::t_str_rdonly> set_names, const zcl::t_array_rdonly<zcl::t_f32> set_f32s);

zcl::t_i32 OptionGetValueIndex(const t_options *const opts, const t_option_id id);

void OptionSetValueIndex(t_options *const opts, const t_option_id id, const zcl::t_i32 index);

zcl::t_i32 OptionGetValueCount(const t_options *const opts, const t_option_id id);

zcl::t_str_rdonly OptionGetValueName(const t_options *const opts, const t_option_id id);
