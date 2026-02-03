#include "world_private.h"

namespace world {
    constexpr zcl::t_i32 k_npc_limit = 1024;
    constexpr zcl::t_v2 k_npc_origin = zcl::k_origin_center;
    constexpr zcl::t_i32 k_npc_flash_time_limit = 10;

    struct t_npc {
        zcl::t_v2 pos;

        zcl::t_i32 health;
        zcl::t_i32 health_limit; // This is in here because it could be randomized even across NPCs of the same type.

        zcl::t_i32 flash_time;

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

        manager->buf[index] = {
            .pos = pos,
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

    void NPCHurt(t_npc_manager *const manager, const t_npc_id id, const zcl::t_i32 damage) {
        ZCL_ASSERT(NPCCheckExists(manager, id));

        const auto npc = &manager->buf[id.index];
        npc->flash_time = k_npc_flash_time_limit;
    }

    zcl::t_b8 NPCCheckExists(const t_npc_manager *const manager, const t_npc_id id) {
        return zcl::BitsetCheckSet(manager->activity, id.index) && id.version == manager->versions[id.index];
    }

    static zcl::t_v2 NPCGetColliderSize(const zcl::t_v2 pos, const t_npc_type_id type_id) {
        return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect));
    }

    zcl::t_rect_f NPCGetCollider(const zcl::t_v2 pos, const t_npc_type_id type_id) {
        switch (type_id) {
            case ek_npc_type_id_slime:
                return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect)), k_npc_origin);

            case ekm_npc_type_id_cnt:
                ZCL_UNREACHABLE();
        }

        ZCL_UNREACHABLE();
    }

    void NPCsProcessAI(t_npc_manager *const npcs, const t_tilemap *const tilemap) {
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

    void NPCsProcessDeaths(t_npc_manager *const npcs) {
        ZCL_BITSET_WALK_ALL_SET (npcs->activity, i) {
            const auto npc = &npcs->buf[i];

            ZCL_ASSERT(npc->health >= 0);

            if (npc->health == 0) {
                zcl::BitsetUnset(npcs->activity, i);
            }
        }
    }

    void NPCsRender(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets) {
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
