#include "tiles.h"

#include "item_drops.h"

struct t_tilemap_core {
    zcl::t_v2_i size;
    zcl::t_bitset_mut activity;
    zcl::t_array_mut<t_tile_type_id> type_ids;
};

struct t_tilemap {
    t_tilemap_core *core; // @note: Could be alternatively be baked into the struct itself?
    zcl::t_array_mut<zcl::t_i32> tile_damage;
};

t_tilemap_core *TilemapCoreCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_tilemap_core>(arena);
    result->size = size;
    result->activity = zcl::BitsetCreate(size.x * size.y, arena);
    result->type_ids = zcl::ArenaPushArray<t_tile_type_id>(arena, size.x * size.y);

    return result;
}

void TilemapCoreAdd(t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type_id) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap_core, tile_pos));
    ZCL_ASSERT(!TilemapCheck(tilemap_core, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap_core->size.x) + tile_pos.x;
    zcl::BitsetSet(tilemap_core->activity, tile_index);
    tilemap_core->type_ids[tile_index] = tile_type_id;
}

void TilemapCoreRemove(t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap_core, tile_pos));
    ZCL_ASSERT(TilemapCheck(tilemap_core, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap_core->size.x) + tile_pos.x;
    zcl::BitsetUnset(tilemap_core->activity, tile_index);
}

t_tilemap *TilemapCreate(t_tilemap_core *const core, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_tilemap>(arena);
    result->core = core;
    result->tile_damage = zcl::ArenaPushArray<zcl::t_i32>(arena, core->size.x * core->size.y);

    return result;
}

void TilemapPlace(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type_id) {
    TilemapCoreAdd(tilemap->core, tile_pos, tile_type_id);

    const zcl::t_i32 tile_index = (tilemap->core->size.x * tile_pos.y) + tile_pos.x;
    tilemap->tile_damage[tile_index] = 0;
}

void TilemapHurt(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage, t_item_drop_manager *const item_drop_manager) {
    ZCL_ASSERT(damage > 0);

    const zcl::t_i32 tile_index = (tilemap->core->size.x * tile_pos.y) + tile_pos.x;
    const auto tile_type = &k_tile_types[tilemap->core->type_ids[tile_index]];

    tilemap->tile_damage[tile_index] += damage;

    if (tilemap->tile_damage[tile_index] >= tile_type->health) {
        TilemapCoreRemove(tilemap->core, tile_pos);
        ItemDropSpawn(item_drop_manager, {(tile_pos.x + 0.5f) * k_tile_size, (tile_pos.y + 0.5f) * k_tile_size}, tile_type->drop_item_type_id, 1);
    }
}

void TilemapRender(const t_tilemap *const tilemap, const zgl::t_rendering_context rc, const zcl::t_rect_i tilemap_subset, const t_assets *const assets) {
    ZCL_ASSERT(zcl::CheckRectEntirelyInRect(tilemap_subset, zcl::RectCreateI(0, 0, tilemap->core->size.x, tilemap->core->size.y)));

    for (zcl::t_i32 ty = zcl::RectGetTop(tilemap_subset); ty < zcl::RectGetBottom(tilemap_subset); ty++) {
        for (zcl::t_i32 tx = zcl::RectGetLeft(tilemap_subset); tx < zcl::RectGetRight(tilemap_subset); tx++) {
            if (!TilemapCheck(tilemap, {tx, ty})) {
                continue;
            }

            const t_tile_type_id tile_type_id = tilemap->core->type_ids[(ty * tilemap->core->size.x) + tx];
            const t_tile_type *const tile_type = &k_tile_types[tile_type_id];

            const zcl::t_v2 tile_render_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

            SpriteRender(tile_type->sprite, rc, assets, tile_render_pos);

            const auto tile_damage = tilemap->tile_damage[(ty * tilemap->core->size.x) + tx];

            if (tile_damage > 0) {
                const zcl::t_f32 tile_damage_perc = static_cast<zcl::t_f32>(tile_damage) / k_tile_types[tile_type_id].health;

                ZCL_ASSERT(tile_damage > 0);
                const zcl::t_i32 tile_hurt_frame_index = zcl::Floor(tile_damage_perc * 4); // @temp: Once animation system is in place, magic number can be dropped.

                SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_tile_hurt_0 + tile_hurt_frame_index), rc, assets, tile_render_pos);
            }
        }
    }
}

t_tilemap_core *TilemapGetCore(t_tilemap *const tilemap) {
    return tilemap->core;
}

const t_tilemap_core *TilemapGetCore(const t_tilemap *const tilemap) {
    return tilemap->core;
}

zcl::t_b8 TilemapCheck(const t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap_core, tile_pos));
    return zcl::BitsetCheckSet(tilemap_core->activity, (tile_pos.y * tilemap_core->size.x) + tile_pos.x);
}

zcl::t_v2_i TilemapGetSize(const t_tilemap_core *const tilemap_core) {
    return tilemap_core->size;
}

zcl::t_b8 TilemapCheckTilePosInBounds(const t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(tilemap_core->size.x > 0 && tilemap_core->size.y > 0);
    return tile_pos.x >= 0 && tile_pos.x < tilemap_core->size.x && tile_pos.y >= 0 && tile_pos.y < tilemap_core->size.y;
}

zcl::t_rect_i TilemapCalcRectSpan(const t_tilemap_core *const tilemap_core, const zcl::t_rect_f rect) {
    const zcl::t_i32 left = static_cast<zcl::t_i32>(zcl::Floor(rect.x / k_tile_size));
    const zcl::t_i32 top = static_cast<zcl::t_i32>(zcl::Floor(rect.y / k_tile_size));
    const zcl::t_i32 right = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetRight(rect) / k_tile_size));
    const zcl::t_i32 bottom = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetBottom(rect) / k_tile_size));

    const zcl::t_rect_i result_without_clamp = {
        left,
        top,
        right - left,
        bottom - top,
    };

    return zcl::ClampWithinContainer(result_without_clamp, zcl::RectCreateI({}, tilemap_core->size));
}

zcl::t_b8 TilemapCheckCollision(const t_tilemap_core *const tilemap_core, const zcl::t_rect_f collider) {
    const zcl::t_rect_i collider_tilemap_span = TilemapCalcRectSpan(tilemap_core, collider);

    for (zcl::t_i32 ty = zcl::RectGetTop(collider_tilemap_span); ty < zcl::RectGetBottom(collider_tilemap_span); ty++) {
        for (zcl::t_i32 tx = zcl::RectGetLeft(collider_tilemap_span); tx < zcl::RectGetRight(collider_tilemap_span); tx++) {
            if (!TilemapCheck(tilemap_core, {tx, ty})) {
                continue;
            }

            const zcl::t_rect_f tile_collider = {
                static_cast<zcl::t_f32>(k_tile_size * tx),
                static_cast<zcl::t_f32>(k_tile_size * ty),
                k_tile_size,
                k_tile_size,
            };

            if (zcl::CheckInters(collider, tile_collider)) {
                return true;
            }
        }
    }

    return false;
}
