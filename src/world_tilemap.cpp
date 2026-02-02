#include "world_private.h"

// @todo: At some point might want to split between serializable and world-relevant state.
// @todo: Maybe activity bits could be removed - just rely on the life values to determine activity?

struct t_tilemap {
    zcl::t_static_bitset<k_tilemap_size.x * k_tilemap_size.y> activity;
    zcl::t_static_array<zcl::t_static_array<t_tile_type_id, k_tilemap_size.x>, k_tilemap_size.y> types;
    zcl::t_static_array<zcl::t_static_array<zcl::t_u8, k_tilemap_size.x>, k_tilemap_size.y> lifes;
};

t_tilemap *TilemapCreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_tilemap>(arena);
}

zcl::t_b8 TilemapPosCheckInBounds(const zcl::t_v2_i pos) {
    return pos.x >= 0 && pos.x < k_tilemap_size.x && pos.y >= 0 && pos.y < k_tilemap_size.y;
}

void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
    ZCL_ASSERT(TilemapPosCheckInBounds(tile_pos));

    const zcl::t_i32 activity_bit_index = (tile_pos.y * k_tilemap_size.x) + tile_pos.x;
    ZCL_ASSERT(!zcl::BitsetCheckSet(tm->activity, activity_bit_index));
    zcl::BitsetSet(tm->activity, activity_bit_index);

    tm->types[tile_pos.y][tile_pos.x] = tile_type;
    tm->lifes[tile_pos.y][tile_pos.x] = k_tile_life_limit;
}

void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapPosCheckInBounds(tile_pos));

    const zcl::t_i32 activity_bit_index = (tile_pos.y * k_tilemap_size.x) + tile_pos.x;
    ZCL_ASSERT(zcl::BitsetCheckSet(tm->activity, activity_bit_index));
    zcl::BitsetUnset(tm->activity, activity_bit_index);
}

void TilemapHurt(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage) {
    ZCL_ASSERT(TilemapPosCheckInBounds(tile_pos));
    ZCL_ASSERT(damage > 0);

    const zcl::t_i32 activity_bit_index = (tile_pos.y * k_tilemap_size.x) + tile_pos.x;
    ZCL_ASSERT(zcl::BitsetCheckSet(tm->activity, activity_bit_index));

    const zcl::t_i32 damage_to_apply = zcl::CalcMin(damage, static_cast<zcl::t_i32>(tm->lifes[tile_pos.y][tile_pos.x]));

    tm->lifes[tile_pos.y][tile_pos.x] -= damage_to_apply;

    if (tm->lifes[tile_pos.y][tile_pos.x] == 0) {
        zcl::BitsetUnset(tm->activity, activity_bit_index);
    }
}

zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapPosCheckInBounds(tile_pos));
    return zcl::BitsetCheckSet(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
}

zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect) {
    const zcl::t_i32 left = static_cast<zcl::t_i32>(floor(rect.x / k_tile_size));
    const zcl::t_i32 top = static_cast<zcl::t_i32>(floor(rect.y / k_tile_size));
    const zcl::t_i32 right = static_cast<zcl::t_i32>(ceil(zcl::RectGetRight(rect) / k_tile_size));
    const zcl::t_i32 bottom = static_cast<zcl::t_i32>(ceil(zcl::RectGetBottom(rect) / k_tile_size));

    const zcl::t_rect_i result_without_clamp = {
        left,
        top,
        right - left,
        bottom - top,
    };

    return zcl::ClampWithinContainer(result_without_clamp, zcl::RectCreateI({}, k_tilemap_size));
}

zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider) {
    const zcl::t_rect_i collider_tilemap_span = TilemapCalcRectSpan(collider);

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

void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets) {
    ZCL_ASSERT(zcl::CheckRectInRect(tm_subset, zcl::RectCreateI(0, 0, k_tilemap_size.x, k_tilemap_size.y)));

    for (zcl::t_i32 ty = zcl::RectGetTop(tm_subset); ty < zcl::RectGetBottom(tm_subset); ty++) {
        for (zcl::t_i32 tx = zcl::RectGetLeft(tm_subset); tx < zcl::RectGetRight(tm_subset); tx++) {
            if (!TilemapCheck(tm, {tx, ty})) {
                continue;
            }

            const t_tile_type_id tile_type_id = tm->types[ty][tx];
            const t_tile_type *const tile_type_info = &k_tile_types[tile_type_id];

            const zcl::t_v2 tile_world_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

            SpriteRender(tile_type_info->sprite, rc, assets, tile_world_pos);
        }
    }
}
