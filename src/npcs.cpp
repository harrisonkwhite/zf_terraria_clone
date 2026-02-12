#include "npcs.h"

#include "assets.h"
#include "tiles.h"
#include "stray.h"

t_npc_id NPCSpawn(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id) {
    const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

    if (index == -1) {
        ZCL_FATAL();
    }

    zcl::BitsetSet(manager->activity, index);

    manager->buf[index] = {
        .health = g_npc_types[type_id].health_limit,
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
    npc->flash_time = k_npc_flash_duration;
}

zcl::t_b8 NPCCheckExists(const t_npc_manager *const manager, const t_npc_id id) {
    return zcl::BitsetCheckSet(manager->activity, id.index) && id.version == manager->versions[id.index];
}

static zcl::t_v2 NPCGetColliderSize(const zcl::t_v2 pos, const t_npc_type_id type_id) {
    return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect));
}

zcl::t_rect_f NPCGetCollider(const zcl::t_v2 pos, const t_npc_type_id type_id) {
    switch (type_id) {
        case ek_npc_type_id_slime: {
            return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect)), k_npc_origin);
        }

        case ekm_npc_type_id_cnt: {
            ZCL_UNREACHABLE();
        }
    }

    ZCL_UNREACHABLE();
}

void NPCsProcessAIs(t_npc_manager *const manager, const zcl::t_f32 gravity, const t_tilemap *const tilemap) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

        const auto npc = &manager->buf[i];

        switch (npc->type_id) {
            case ek_npc_type_id_slime: {
                const auto slime = &npc->type_data.slime;

                slime->vel.y += gravity;

                ProcessTilemapCollisions(&npc->pos, &slime->vel, NPCGetColliderSize(npc->pos, npc->type_id), k_npc_origin, tilemap);

                npc->pos += slime->vel;

                break;
            }

            case ekm_npc_type_id_cnt: {
                ZCL_UNREACHABLE();
            }
        }
    }
}

void NPCsProcessDeaths(t_npc_manager *const manager) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

        const auto npc = &manager->buf[i];

        ZCL_ASSERT(npc->health >= 0);

        if (npc->health == 0) {
            zcl::BitsetUnset(manager->activity, i);
        }
    }
}

void NPCsRender(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

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

zcl::t_array_mut<t_npc *> NPCsLoad(t_npc_manager *const manager, zcl::t_arena *const arena) {
    const zcl::t_i32 npc_cnt = zcl::BitsetCountSet(manager->activity);

    const auto result = zcl::ArenaPushArray<t_npc *>(arena, npc_cnt);

    for (zcl::t_i32 i = 0, result_index = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

        result[result_index] = &manager->buf[i];
        result_index++;
    }

    return result;
}

zcl::t_array_rdonly<t_npc *> NPCsLoad(const t_npc_manager *const manager, zcl::t_arena *const arena) {
    return NPCsLoad(const_cast<t_npc_manager *>(manager), arena);
}
