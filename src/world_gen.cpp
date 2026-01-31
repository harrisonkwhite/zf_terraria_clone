#include "world_private.h"

t_tilemap *WorldGen(zcl::t_rng *const rng, zcl::t_arena *const arena) {
    const auto tilemap = TilemapCreate(arena);

    zcl::t_static_array<zcl::t_i32, k_tilemap_size.x> ground_offsets;

    constexpr zcl::t_i32 k_ground_height = 10;

    zcl::t_i32 ground_offs_pen = zcl::RandGenI32InRange(rng, 0, k_ground_height);

    for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
        ground_offsets[x] = ground_offs_pen;

        if (zcl::RandGenPerc(rng) < 0.3f) {
            const zcl::t_b8 down = (ground_offs_pen == 0 || zcl::RandGenPerc(rng) < 0.5f) && ground_offs_pen < k_ground_height - 1;
            ground_offs_pen += down ? 1 : -1;
        }
    }

    const zcl::t_i32 ground_tilemap_y_begin = k_tilemap_size.y / 3.0f;

    for (zcl::t_i32 gy = 0; gy < k_ground_height; gy++) {
        for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
            if (gy >= ground_offsets[x]) {
                TilemapAdd(tilemap, {x, ground_tilemap_y_begin + gy}, ek_tile_type_id_dirt);
            }
        }
    }

    for (zcl::t_i32 y = ground_tilemap_y_begin + k_ground_height; y < k_tilemap_size.y; y++) {
        for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
            TilemapAdd(tilemap, {x, y}, ek_tile_type_id_dirt);
        }
    }

    return tilemap;
}
