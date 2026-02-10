#include "tilemaps.h"

struct t_tilemap {
    zcl::t_v2_i size;
    zcl::t_bitset_mut activity;
    zcl::t_array_mut<t_tile_type_id> type_ids;
};

t_tilemap *TilemapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_tilemap>(arena);
    result->activity = zcl::BitsetCreate(size.x * size.y, arena);
    result->type_ids = zcl::ArenaPushArray<t_tile_type_id>(arena, size.x * size.y);

    return result;
}

zcl::t_b8 TilemapCheck(const t_tilemap *const tilemap, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap->size, tile_pos));
    return zcl::BitsetCheckSet(tilemap->activity, (tile_pos.y * tilemap->size.x) + tile_pos.y);
}

void TilemapAdd(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap->size, tile_pos));
    ZCL_ASSERT(!TilemapCheck(tilemap, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap->size.x) + tile_pos.x;
    zcl::BitsetSet(tilemap->activity, tile_index);
    tilemap->type_ids[tile_index] = tile_type;
}

void TilemapRemove(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
    ZCL_ASSERT(TilemapCheckTilePosInBounds(tilemap->size, tile_pos));
    ZCL_ASSERT(!TilemapCheck(tilemap, tile_pos));

    const zcl::t_i32 tile_index = (tile_pos.y * tilemap->size.x) + tile_pos.x;
    zcl::BitsetUnset(tilemap->activity, tile_index);
}

zcl::t_b8 TilemapCheckTilePosInBounds(const zcl::t_v2_i tilemap_size, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(tilemap_size.x > 0 && tilemap_size.y > 0);
    return tile_pos.x >= 0 && tile_pos.x < tilemap_size.x && tile_pos.y >= 0 && tile_pos.y < tilemap_size.y;
}
