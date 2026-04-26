#include "world_gen.h"

#include "tiles.h"

t_tilemap_core *WorldGen(const zcl::t_v2_i size, zcl::t_rng *const rng, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    const auto tilemap = TilemapCoreCreate(size, arena);

    const auto calc_lvl_offsets = [size, rng, temp_arena](const zcl::t_i32 variation_range, const zcl::t_f32 variation_chance) {
        ZCL_ASSERT(variation_chance >= 0.0f && variation_chance <= 1.0f);

        const auto result = zcl::ArenaPushArray<zcl::t_i32>(temp_arena, size.x);

        {
            zcl::t_i32 offs_pen = zcl::RandGenI32InRange(rng, 0, variation_range);

            for (zcl::t_i32 x = 0; x < size.x; x++) {
                result[x] = offs_pen;

                if (zcl::RandGenPerc(rng) < variation_chance) {
                    const zcl::t_b8 down = (offs_pen == 0 || zcl::RandGenPerc(rng) < 0.5f) && offs_pen < variation_range - 1;
                    offs_pen += down ? 1 : -1;
                }
            }
        }

        return result;
    };

    const zcl::t_i32 dirt_lvl_base = size.y / 3;
    const auto dirt_lvl_offsets = calc_lvl_offsets(8, 0.3f);

    constexpr zcl::t_i32 k_dirt_lvl_grass_depth = 3;

    const zcl::t_i32 stone_lvl_base_rel_to_dirt_lvl = 20;
    const auto stone_lvl_offsets = calc_lvl_offsets(6, 0.5f);

    for (zcl::t_i32 y = dirt_lvl_base; y < size.y; y++) {
        for (zcl::t_i32 x = 0; x < size.x; x++) {
            const zcl::t_i32 dirt_lvl = dirt_lvl_base + dirt_lvl_offsets[x];
            const zcl::t_i32 stone_lvl = dirt_lvl_base + stone_lvl_base_rel_to_dirt_lvl + stone_lvl_offsets[x];

            if (y >= stone_lvl) {
                TilemapCoreAdd(tilemap, {x, y}, ek_tile_type_id_stone);
            } else if (y >= dirt_lvl + k_dirt_lvl_grass_depth) {
                TilemapCoreAdd(tilemap, {x, y}, ek_tile_type_id_dirt);
            } else if (y >= dirt_lvl) {
                TilemapCoreAdd(tilemap, {x, y}, ek_tile_type_id_grass);
            }
        }
    }

    return tilemap;
}
