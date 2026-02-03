#include "world_private.h"

#include "tilemaps.h"

namespace world {
    t_npc_id SpawnNPC(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id) {
        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

        if (index == -1) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(manager->activity, index);

        manager->buf[index] = {
            .pos = pos,
            .health = g_npc_types[type_id].health_limit,
            .type_id = type_id,
        };

        switch (type_id) {
            case ek_npc_type_id_slime: {
                break;
            }

            case ekm_npc_type_id_cnt: {
                ZCL_UNREACHABLE();
            }
        }

        return {index, manager->versions[index]};
    }

    void HurtNPC(t_npc_manager *const manager, const t_npc_id id, const zcl::t_i32 damage) {
        ZCL_ASSERT(CheckNPCExists(manager, id));

        const auto npc = &manager->buf[id.index];
        npc->flash_time = k_npc_flash_time_limit;
    }

    zcl::t_b8 CheckNPCExists(const t_npc_manager *const manager, const t_npc_id id) {
        return zcl::BitsetCheckSet(manager->activity, id.index) && id.version == manager->versions[id.index];
    }

    static zcl::t_v2 NPCGetColliderSize(const zcl::t_v2 pos, const t_npc_type_id type_id) {
        return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect));
    }

    zcl::t_rect_f GetNPCCollider(const zcl::t_v2 pos, const t_npc_type_id type_id) {
        switch (type_id) {
            case ek_npc_type_id_slime:
                return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect)), k_npc_origin);

            case ekm_npc_type_id_cnt:
                ZCL_UNREACHABLE();
        }

        ZCL_UNREACHABLE();
    }

    void ProcessNPCAIs(t_npc_manager *const npcs, const t_tilemap *const tilemap) {
        ZCL_BITSET_WALK_ALL_SET (npcs->activity, i) {
            const auto npc = &npcs->buf[i];

            switch (npc->type_id) {
                case ek_npc_type_id_slime: {
                    const auto slime = &npc->type_data.slime;

                    slime->vel.y += k_gravity;

                    TilemapProcessCollisions(tilemap, &npc->pos, &slime->vel, NPCGetColliderSize(npc->pos, npc->type_id), k_npc_origin);

                    npc->pos += slime->vel;

                    break;
                }

                case ekm_npc_type_id_cnt: {
                    ZCL_UNREACHABLE();
                }
            }
        }
    }

    void ProcessNPCDeaths(t_npc_manager *const npcs) {
        ZCL_BITSET_WALK_ALL_SET (npcs->activity, i) {
            const auto npc = &npcs->buf[i];

            ZCL_ASSERT(npc->health >= 0);

            if (npc->health == 0) {
                zcl::BitsetUnset(npcs->activity, i);
            }
        }
    }

    void RenderNPCs(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets) {
        ZCL_BITSET_WALK_ALL_SET (manager->activity, i) {
            const auto npc = &manager->buf[i];

            switch (npc->type_id) {
                case ek_npc_type_id_slime: {
                    SpriteRender(ek_sprite_id_npc_slime, rc, assets, npc->pos, zcl::k_origin_center);
                    break;
                }

                case ekm_npc_type_id_cnt: {
                    ZCL_UNREACHABLE();
                }
            }
        }
    }
}
