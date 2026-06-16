#include "item_drops.h"

#include "player.h"
#include "inventories.h"
#include "stray.h"
#include "pop_ups.h"

constexpr zcl::t_i32 k_item_drop_limit = 1024;
constexpr zcl::t_f32 k_item_drop_item_type_icon_scale = 0.5f;
constexpr zcl::t_v2 k_item_drop_origin = {0.5f, 0.5f};

struct t_item_drop {
    zcl::t_v2 pos;
    zcl::t_v2 vel;

    t_item_type_id item_type_id;
    zcl::t_i32 item_quantity;
};

struct t_item_drop_manager {
    zcl::t_static_array<t_item_drop, k_item_drop_limit> buf;
    zcl::t_static_bitset<k_item_drop_limit> activity;
};

t_item_drop_manager *ItemDropManagerCreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_item_drop_manager>(arena);
}

void ItemDropSpawn(t_item_drop_manager *const drop_manager, const zcl::t_v2 pos, const t_item_type_id item_type_id, const zcl::t_i32 item_quantity) {
    ZCL_ASSERT(item_quantity > 0);

    const zcl::t_i32 index = zcl::BitsetFindFirstUnset(drop_manager->activity);

    if (index == -1) {
        ZCL_FATAL();
    }

    zcl::BitsetSet(drop_manager->activity, index);

    drop_manager->buf[index] = {
        .pos = pos,
        .item_type_id = item_type_id,
        .item_quantity = item_quantity,
    };
}

static zcl::t_v2 GetItemDropColliderSize(const t_item_type_id item_type_id) {
    return zcl::V2IToF(zcl::RectGetSize(k_sprites[g_item_types[item_type_id].icon_sprite_id].src_rect)) * k_item_drop_item_type_icon_scale;
}

zcl::t_rect_f GetItemDropCollider(const zcl::t_v2 pos, const t_item_type_id item_type_id) {
    return ColliderCreate(pos, GetItemDropColliderSize(item_type_id), k_item_drop_origin);
}

void ItemDropsProcessMovementAndCollection(t_item_drop_manager *const drop_manager, t_player_meta *const player_meta, const t_player_entity *const player_entity, const zcl::t_f32 gravity, const t_tilemap *const tilemap, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
    // ----------------------------------------
    // Movement

    for (zcl::t_i32 i = 0; i < k_item_drop_limit; i++) {
        if (!zcl::BitsetCheckSet(drop_manager->activity, i)) {
            continue;
        }

        const auto drop = &drop_manager->buf[i];

        drop->vel.y += gravity;

        ProcessTilemapCollisions(&drop->pos, &drop->vel, GetItemDropColliderSize(drop->item_type_id), k_item_drop_origin, tilemap);

        drop->pos += drop->vel;
    }

    // ------------------------------

    // ----------------------------------------
    // Player Collection

    if (PlayerCheckAlive(player_entity)) {
        const auto player_collider = PlayerGetCollider(PlayerGetPosition(player_entity));

        for (zcl::t_i32 i = 0; i < k_item_drop_limit; i++) {
            if (!zcl::BitsetCheckSet(drop_manager->activity, i)) {
                continue;
            }

            const auto drop = &drop_manager->buf[i];

            const auto item_drop_collider = GetItemDropCollider(drop->pos, drop->item_type_id);

            if (zcl::CheckInters(player_collider, item_drop_collider)) {
                InventoryAdd(PlayerGetInventory(player_meta), drop->item_type_id, drop->item_quantity);

                const zcl::t_v2 pop_up_vel = zcl::k_cardinal_direction_normals[zcl::ek_cardinal_direction_up] * zcl::RandGenF32InRange(rng, 5.5f, 6.0f);
                const auto pop_up = PopUpSpawn(pop_up_manager, 90, drop->pos, pop_up_vel);

                const auto item_str = InventoryDetermineItemStr(drop->item_type_id, drop->item_quantity, temp_arena);
                pop_up->str_byte_cnt = zcl::ArrayCopy(item_str.bytes, zcl::ArrayToNonstatic(&pop_up->str_bytes), true); // @speed: Could just write it directly to the buffer instead...

                zcl::BitsetUnset(drop_manager->activity, i);
            }
        }
    }

    // ------------------------------
}

void ItemDropsRender(const t_item_drop_manager *const drop_manager, const zgl::t_rendering_context rc, const t_assets *const assets) {
    for (zcl::t_i32 i = 0; i < k_item_drop_limit; i++) {
        if (!zcl::BitsetCheckSet(drop_manager->activity, i)) {
            continue;
        }

        const auto item_drop = &drop_manager->buf[i];
        SpriteRender(g_item_types[item_drop->item_type_id].icon_sprite_id, rc, assets, item_drop->pos, k_item_drop_origin, 0.0f, {k_item_drop_item_type_icon_scale, k_item_drop_item_type_icon_scale});
    }
}
