#include "world_private.h"

namespace world {
    void SpawnItemDrop(t_item_drop_manager *const manager, const zcl::t_v2 pos, const t_item_type_id item_type_id) {
        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

        if (index == -1) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(manager->activity, index);

        manager->buf[index] = {
            .pos = pos,
            .item_type_id = item_type_id,
        };
    }

    static zcl::t_v2 GetItemDropColliderSize(const t_item_type_id item_type_id) {
        return zcl::V2IToF(zcl::RectGetSize(k_sprites[g_item_types[item_type_id].icon_sprite_id].src_rect)) * k_item_drop_item_type_icon_scale;
    }

    zcl::t_rect_f GetItemDropCollider(const zcl::t_v2 pos, const t_item_type_id item_type_id) {
        return ColliderCreate(pos, GetItemDropColliderSize(item_type_id), k_item_drop_origin);
    }

    void ProcessItemDropMovementAndCollection(t_item_drop_manager *const item_drop_manager, t_player_meta *const player_meta, const t_player_entity *const player_entity, const t_tilemap *const tilemap) {
        ZCL_BITSET_WALK_ALL_SET (item_drop_manager->activity, i) {
            const auto item_drop = &item_drop_manager->buf[i];

            item_drop->vel.y += k_gravity;

            TilemapProcessCollisions(tilemap, &item_drop->pos, &item_drop->vel, GetItemDropColliderSize(item_drop->item_type_id), k_item_drop_origin);

            item_drop->pos += item_drop->vel;
        }
    }

    void RenderItemDrops(const zgl::t_rendering_context rc, const t_item_drop_manager *const item_drop_manager, const t_assets *const assets) {
        ZCL_BITSET_WALK_ALL_SET (item_drop_manager->activity, i) {
            const auto item_drop = &item_drop_manager->buf[i];
            SpriteRender(g_item_types[item_drop->item_type_id].icon_sprite_id, rc, assets, item_drop->pos, k_item_drop_origin, 0.0f, {k_item_drop_item_type_icon_scale, k_item_drop_item_type_icon_scale});
        }
    }
}
