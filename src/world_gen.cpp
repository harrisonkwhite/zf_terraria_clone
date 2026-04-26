#include "world_gen.h"

#include "tiles.h"

t_tilemap_core *WorldGen(const zcl::t_v2_i size, zcl::t_rng *const rng, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    const auto tilemap = TilemapCoreCreate(size, arena);

    const auto calc_lvls = [size, rng, temp_arena](const zcl::t_i32 base, const zcl::t_i32 variation_range, const zcl::t_f32 variation_chance) {
        ZCL_ASSERT(variation_chance >= 0.0f && variation_chance <= 1.0f);

        const auto result = zcl::ArenaPushArray<zcl::t_i32>(temp_arena, size.x);

        {
            zcl::t_i32 offs_pen = zcl::RandGenI32InRange(rng, 0, variation_range);

            for (zcl::t_i32 x = 0; x < size.x; x++) {
                result[x] = base + offs_pen;

                if (zcl::RandGenPerc(rng) < variation_chance) {
                    const zcl::t_b8 down = (offs_pen == 0 || zcl::RandGenPerc(rng) < 0.5f) && offs_pen < variation_range - 1;
                    offs_pen += down ? 1 : -1;
                }
            }
        }

        return result;
    };

    const auto dirt_lvls = calc_lvls(size.y / 3, 10, 0.3f);
    const auto stone_lvls = calc_lvls((size.y / 3) + 20, 5, 0.5f);

    for (zcl::t_i32 y = 0; y < size.y; y++) {
        for (zcl::t_i32 x = 0; x < size.x; x++) {
            if (y >= stone_lvls[x]) {
                TilemapCoreAdd(tilemap, {x, y}, ek_tile_type_id_stone);
            } else if (y > dirt_lvls[x] + 2) {
                TilemapCoreAdd(tilemap, {x, y}, ek_tile_type_id_dirt);
            } else if (y >= dirt_lvls[x]) {
                TilemapCoreAdd(tilemap, {x, y}, ek_tile_type_id_grass);
            }
        }
    }

    // static_assert(false, "dodgy above!");

    return tilemap;
}
