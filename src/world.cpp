#include "world.h"


// ============================================================
// @section: Tiles and Tilemap
// ============================================================

enum t_tile_type : zcl::t_i8 {
    ek_tile_type_dirt,
    ek_tile_type_stone,
    ek_tile_type_grass,
    ek_tile_type_sand,

    ekm_tile_type_cnt
};

constexpr zcl::t_v2_i k_tilemap_size = {100, 100};

struct t_tilemap {
    zcl::t_static_bitset<k_tilemap_size.x * k_tilemap_size.y> activity;
    zcl::t_static_array<zcl::t_static_array<t_tile_type, k_tilemap_size.x>, k_tilemap_size.y> types;
};

static zcl::t_b8 TilePosCheckInBounds(const zcl::t_v2_i pos) {
    return pos.x >= 0 && pos.x < k_tilemap_size.x && pos.y >= 0 && pos.y < k_tilemap_size.y;
}

static void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type tile_type) {
    ZCL_ASSERT(TilePosCheckInBounds(tile_pos));

    zcl::BitsetSet(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
    tm->types[tile_pos.y][tile_pos.x] = tile_type;
}

static void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilePosCheckInBounds(tile_pos));
    zcl::BitsetUnset(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
}

// ============================================================


struct t_world {
    t_tilemap tilemap;
};

t_world *WorldCreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_world>(arena);
}

void WorldTick(t_world *const world) {
}

void WorldRender(t_world *const world) {
}
