#include "tiles.h"

struct t_tilemap {
    zcl::t_v2_i size;
    zcl::t_bitset_mut activity;
    zcl::t_array_mut<t_tile_type_id> type_ids;
};

t_tilemap *TilemapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_tilemap>(arena);
    result->size = size;
    result->activity = zcl::BitsetCreate(size.x * size.y, arena);
    result->type_ids = zcl::ArenaPushArray<t_tile_type_id>(arena, size.x * size.y);

    return result;
}

zcl::t_b8 TilemapCheck(const t_tilemap *const tilemap, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap, tile_pos));
    return zcl::BitsetCheckSet(tilemap->activity, (tile_pos.y * tilemap->size.x) + tile_pos.x);
}

void TilemapAdd(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap, tile_pos));
    ZCL_ASSERT(!TilemapCheck(tilemap, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap->size.x) + tile_pos.x;
    zcl::BitsetSet(tilemap->activity, tile_index);
    tilemap->type_ids[tile_index] = tile_type;
}

void TilemapRemove(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap, tile_pos));
    ZCL_ASSERT(!TilemapCheck(tilemap, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap->size.x) + tile_pos.x;
    zcl::BitsetUnset(tilemap->activity, tile_index);
}

zcl::t_v2_i TilemapGetSize(const t_tilemap *const tilemap) {
    return tilemap->size;
}

zcl::t_b8 TilemapCheckTilePosInBounds(const t_tilemap *const tilemap, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(tilemap->size.x > 0 && tilemap->size.y > 0);
    return tile_pos.x >= 0 && tile_pos.x < tilemap->size.x && tile_pos.y >= 0 && tile_pos.y < tilemap->size.y;
}

zcl::t_rect_i TilemapCalcRectSpan(const t_tilemap *const tilemap, const zcl::t_rect_f rect) {
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

    return zcl::ClampWithinContainer(result_without_clamp, zcl::RectCreateI({}, tilemap->size));
}

zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider) {
    const zcl::t_rect_i collider_tilemap_span = TilemapCalcRectSpan(tilemap, collider);

    for (zcl::t_i32 ty = zcl::RectGetTop(collider_tilemap_span); ty < zcl::RectGetBottom(collider_tilemap_span); ty++) {
        for (zcl::t_i32 tx = zcl::RectGetLeft(collider_tilemap_span); tx < zcl::RectGetRight(collider_tilemap_span); tx++) {
            if (!TilemapCheck(tilemap, {tx, ty})) {
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
