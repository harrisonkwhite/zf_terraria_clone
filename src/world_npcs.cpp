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

    struct t_npc_manager {
        zcl::t_static_array<t_npc, k_npc_limit> buf;
        zcl::t_static_bitset<k_npc_limit> activity;
        zcl::t_static_array<zcl::t_i32, k_npc_limit> versions;
    };

    t_npc_manager *NPCManagerCreate(zcl::t_arena *const arena) {
        return zcl::ArenaPush<t_npc_manager>(arena);
    }

    t_npc_id NPCSpawn(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id) {
        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

        if (index == -1) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(manager->activity, index);

        return {index, manager->versions[index]};
    }

    zcl::t_b8 NPCCheckExists(const t_npc_manager *const manager, const t_npc_id id) {
        return zcl::BitsetCheckSet(manager->activity, id.index) && id.version == manager->versions[id.index];
    }

    void NPCsUpdate(t_npc_manager *const npcs, const t_tilemap *const tilemap) {
        ZCL_BITSET_WALK_ALL_SET (npcs->activity, i) {
            const auto npc = &npcs->buf[i];

            switch (npc->type_id) {
                case ek_npc_type_id_slime:
                    break;

                case ekm_npc_type_id_cnt:
                    ZCL_UNREACHABLE();
            }
        }
    }

    void NPCsRender(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets) {
        ZCL_BITSET_WALK_ALL_SET (manager->activity, i) {
            const auto npc = &manager->buf[i];

            switch (npc->type_id) {
                case ek_npc_type_id_slime:
                    SpriteRender(ek_sprite_id_player, rc, assets, npc->pos, zcl::k_origin_center);
                    break;

                case ekm_npc_type_id_cnt:
                    ZCL_UNREACHABLE();
            }
        }
    }
}
