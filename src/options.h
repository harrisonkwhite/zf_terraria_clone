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

enum t_option_value_type_id : zcl::t_i32 {
    ek_option_value_type_id_b8,
    ek_option_value_type_id_f32,
};

struct t_option_value_set {
    zcl::t_array_mut<zcl::t_str_rdonly> names;

    t_option_value_type_id value_type_id;

    union {
        zcl::t_array_mut<zcl::t_b8> b8s;
        zcl::t_array_mut<zcl::t_f32> f32s;
    } value_type_data;
};

struct t_options {
    zcl::t_static_array<t_option_value_set, ekm_option_id_cnt> value_sets;
    zcl::t_static_array<zcl::t_i32, ekm_option_id_cnt> value_set_indexes;
};
