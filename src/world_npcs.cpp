#include "world_private.h"

namespace world {
    constexpr zcl::t_i32 k_npc_limit = 1024;

    struct t_npc {
        zcl::t_v2 pos;

        t_npc_type_id type_id;

        union {
            struct {
                zcl::t_v2 vel;
            } slime;
        } type_data;
    };

    struct t_npcs {
        zcl::t_static_array<t_npc, k_npc_limit> buf;
        zcl::t_static_bitset<k_npc_limit> activity;
        zcl::t_static_array<zcl::t_i32, k_npc_limit> versions;
    };

    t_npcs *NPCsCreate(zcl::t_arena *const arena) {
        return zcl::ArenaPush<t_npcs>(arena);
    }

    t_npc_id NPCSpawn(t_npcs *const npcs, const zcl::t_v2 pos, const t_npc_type_id type_id) {
        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(npcs->activity);

        if (index == -1) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(npcs->activity, index);

        return {index, npcs->versions[index]};
    }

    void NPCsUpdate(t_npcs *const npcs, const t_tilemap *const tilemap) {
    }

    void NPCsRender(const t_npcs *const npcs, const zgl::t_rendering_context rc, const t_assets *const assets) {
    }
}
