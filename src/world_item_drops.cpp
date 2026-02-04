#include "world_private.h"

#include "inventories.h"

namespace world {
    void SpawnItemDrop(t_item_drop_manager *const manager, const zcl::t_v2 pos, const t_item_type_id item_type_id, const zcl::t_i32 item_quantity) {
        ZCL_ASSERT(item_quantity > 0);

        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

        if (index == -1) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(manager->activity, index);

        manager->buf[index] = {
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

    void ProcessItemDropMovementAndCollection(t_item_drop_manager *const item_drop_manager, t_player_meta *const player_meta, const t_player_entity *const player_entity, const t_tilemap *const tilemap, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng) {
        const auto player_collider = GetPlayerCollider(player_entity->pos);

        ZCL_BITSET_WALK_ALL_SET (item_drop_manager->activity, i) {
            const auto item_drop = &item_drop_manager->buf[i];

            item_drop->vel.y += k_gravity;

            TilemapProcessCollisions(tilemap, &item_drop->pos, &item_drop->vel, GetItemDropColliderSize(item_drop->item_type_id), k_item_drop_origin);

            item_drop->pos += item_drop->vel;

            const auto item_drop_collider = GetItemDropCollider(item_drop->pos, item_drop->item_type_id);

            if (zcl::CheckInters(player_collider, item_drop_collider)) {
                InventoryAdd(player_meta->inventory, item_drop->item_type_id, item_drop->item_quantity);

                const zcl::t_v2 pop_up_vel = zcl::k_cardinal_direction_normals[zcl::ek_cardinal_direction_up] * zcl::RandGenF32InRange(rng, 5.5f, 6.0f);
                const auto pop_up = SpawnPopUp(pop_up_manager, 90, item_drop->pos, pop_up_vel);
                zcl::t_byte_stream pop_up_str_bytes_stream = zcl::ByteStreamCreate(pop_up->str_bytes, zcl::ek_stream_mode_write);
                zcl::PrintFormat(zcl::ByteStreamGetView(&pop_up_str_bytes_stream), ZCL_STR_LITERAL("% x%"), g_item_types[item_drop->item_type_id].name, item_drop->item_quantity);
                pop_up->str_byte_cnt = zcl::ByteStreamGetWritten(&pop_up_str_bytes_stream).len;

                zcl::BitsetUnset(item_drop_manager->activity, i);
            }
        }
    }

    void RenderItemDrops(const zgl::t_rendering_context rc, const t_item_drop_manager *const item_drop_manager, const t_assets *const assets) {
        ZCL_BITSET_WALK_ALL_SET (item_drop_manager->activity, i) {
            const auto item_drop = &item_drop_manager->buf[i];
            SpriteRender(g_item_types[item_drop->item_type_id].icon_sprite_id, rc, assets, item_drop->pos, k_item_drop_origin, 0.0f, {k_item_drop_item_type_icon_scale, k_item_drop_item_type_icon_scale});
        }
    }
}
