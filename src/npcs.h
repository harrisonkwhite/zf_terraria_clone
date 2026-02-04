#pragma once

#include "sprites.h"

enum t_npc_type_id : zcl::t_i32 {
    ek_npc_type_id_slime,

    ekm_npc_type_id_cnt
};

struct t_npc_type {
    zcl::t_str_rdonly name;

    zcl::t_i32 health_limit;

    zcl::t_b8 touch_hurt;
    zcl::t_i32 touch_hurt_damage;
    zcl::t_f32 touch_hurt_force_mag;
};

inline const zcl::t_static_array<t_npc_type, ekm_npc_type_id_cnt> g_npc_types = {{
    {
        .name = ZCL_STR_LITERAL("Slime"),
        .health_limit = 30,
        .touch_hurt = true,
        .touch_hurt_damage = 10,
    },
}};
