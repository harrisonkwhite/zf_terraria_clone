#pragma once

#include "sprites.h"

enum t_npc_type_id : zcl::t_i32 {
    ek_npc_type_id_slime,

    ekm_npc_type_id_cnt
};

struct t_npc_type {
    zcl::t_str_rdonly name;
};

inline const zcl::t_static_array<t_npc_type, ekm_npc_type_id_cnt> g_npc_types = {{
    {
        .name = ZCL_STR_LITERAL("Slime"),
    },
}};
