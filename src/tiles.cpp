#include "tiles.h"

struct t_tilemap_core {
    zcl::t_v2_i size;
    zcl::t_bitset_mut activity;
    zcl::t_array_mut<t_tile_type_id> type_ids;
};

struct t_untitled {
    t_tilemap_core *tilemap; // @note: Could be alternatively be baked into the struct itself?
    zcl::t_v2_i chunk_size;
};

t_tilemap_core *TilemapCoreCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_tilemap_core>(arena);
    result->size = size;
    result->activity = zcl::BitsetCreate(size.x * size.y, arena);
    result->type_ids = zcl::ArenaPushArray<t_tile_type_id>(arena, size.x * size.y);

    return result;
}

zcl::t_b8 TilemapCheck(const t_tilemap_core *const tilemap, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap, tile_pos));
    return zcl::BitsetCheckSet(tilemap->activity, (tile_pos.y * tilemap->size.x) + tile_pos.x);
}

void TilemapAdd(t_tilemap_core *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap, tile_pos));
    ZCL_ASSERT(!TilemapCheck(tilemap, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap->size.x) + tile_pos.x;
    zcl::BitsetSet(tilemap->activity, tile_index);
    tilemap->type_ids[tile_index] = tile_type;
}

void TilemapRemove(t_tilemap_core *const tilemap, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap, tile_pos));
    ZCL_ASSERT(TilemapCheck(tilemap, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap->size.x) + tile_pos.x;
    zcl::BitsetUnset(tilemap->activity, tile_index);
}

void RenderTilemap(const zgl::t_rendering_context rc, const t_tilemap_core *const tilemap, const zcl::t_rect_i tilemap_subset, const t_assets *const assets) {
    ZCL_ASSERT(zcl::CheckRectInRect(tilemap_subset, zcl::RectCreateI(0, 0, tilemap->size.x, tilemap->size.y)));

    for (zcl::t_i32 ty = zcl::RectGetTop(tilemap_subset); ty < zcl::RectGetBottom(tilemap_subset); ty++) {
        for (zcl::t_i32 tx = zcl::RectGetLeft(tilemap_subset); tx < zcl::RectGetRight(tilemap_subset); tx++) {
            if (!TilemapCheck(tilemap, {tx, ty})) {
                continue;
            }

            const t_tile_type_id tile_type_id = tilemap->type_ids[(ty * tilemap->size.x) + tx];
            const t_tile_type *const tile_type = &k_tile_types[tile_type_id];

            const zcl::t_v2 tile_render_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

            SpriteRender(tile_type->sprite, rc, assets, tile_render_pos);

#if 0
                const auto tile_life = tilemap->lifes[ty][tx];
                const auto tile_type_life = k_tile_types[tile_type_id].life_duration;

                if (tile_life < tile_type_life) {
                    const zcl::t_f32 tile_life_perc_inv = 1.0f - (static_cast<zcl::t_f32>(tile_life) / k_tile_types[tile_type_id].life_duration);

                    ZCL_ASSERT(tile_life > 0);
                    const zcl::t_i32 tile_hurt_frame_index = zcl::Floor(tile_life_perc_inv * 4); // @temp: Once animation system is in place, magic number can be dropped.

                    SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_tile_hurt_0 + tile_hurt_frame_index), rc, assets, tile_render_pos);
                }
#endif
        }
    }
}

zcl::t_v2_i TilemapGetSize(const t_tilemap_core *const tilemap) {
    return tilemap->size;
}

zcl::t_b8 TilemapCheckTilePosInBounds(const t_tilemap_core *const tilemap, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(tilemap->size.x > 0 && tilemap->size.y > 0);
    return tile_pos.x >= 0 && tile_pos.x < tilemap->size.x && tile_pos.y >= 0 && tile_pos.y < tilemap->size.y;
}

zcl::t_rect_i TilemapCalcRectSpan(const t_tilemap_core *const tilemap, const zcl::t_rect_f rect) {
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

zcl::t_b8 TilemapCheckCollision(const t_tilemap_core *const tilemap, const zcl::t_rect_f collider) {
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

t_untitled *TilemapCreate(t_tilemap_core *const tilemap, const zcl::t_v2_i chunk_size, zcl::t_arena *const arena) {
    ZCL_ASSERT(tilemap->size.x % chunk_size.x == 0 && tilemap->size.y % chunk_size.y == 0);

    const auto result = zcl::ArenaPush<t_untitled>(arena);
    result->tilemap = tilemap;
    result->chunk_size = chunk_size;

    return result;
}

t_tilemap_core *TilemapGetCore(t_untitled *const untitled) {
    return untitled->tilemap;
}

const t_tilemap_core *TilemapGetCore(const t_untitled *const untitled) {
    return untitled->tilemap;
}
