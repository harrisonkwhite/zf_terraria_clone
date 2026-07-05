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

struct t_option {
    zcl::t_str_rdonly name;
    t_option_type_id type_id;
};

inline const zcl::t_static_array<t_option, ekm_option_id_cnt> g_options = {{
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
