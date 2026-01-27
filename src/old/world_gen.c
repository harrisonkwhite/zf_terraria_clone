#include "game.h"

#define GROUND_LEVEL_BASE (TILEMAP_HEIGHT / 3)
#define GROUND_LEVEL_OFFS_LIM 8
#define GROUND_LEVEL_TOP (GROUND_LEVEL_BASE - GROUND_LEVEL_OFFS_LIM)
#define GROUND_LEVEL_BOTTOM (GROUND_LEVEL_BASE + GROUND_LEVEL_OFFS_LIM)
static_assert(GROUND_LEVEL_TOP >= 0 && GROUND_LEVEL_BOTTOM <= TILEMAP_HEIGHT, "Tilemap ground level range goes out of bounds!");
#define GROUND_LEVEL_OFFS_VAR 0.4f
static_assert(GROUND_LEVEL_OFFS_VAR >= 0.0f && GROUND_LEVEL_OFFS_VAR <= 1.0f, "Variance is out of range!");

static void GroundGenStep(s_tilemap_core* const tm_core) {
    assert(tm_core);

    t_s32 level = GROUND_LEVEL_BASE + RandRangeS32(-GROUND_LEVEL_OFFS_LIM, GROUND_LEVEL_OFFS_LIM);

    for (t_s32 tx = 0; tx < TILEMAP_WIDTH; tx++) {
        const t_s32 grass_depth = RandRangeS32(2, 4);
        const t_s32 dirt_depth = RandRangeS32(11, 15);
        assert(grass_depth < dirt_depth && level + dirt_depth <= TILEMAP_HEIGHT);

        for (t_s32 ty = level; ty < level + grass_depth; ty++) {
            AddTile(tm_core, (s_v2_s32){tx, ty}, ek_tile_type_grass);
        }

        for (t_s32 ty = level + grass_depth; ty < level + dirt_depth; ty++) {
            AddTile(tm_core, (s_v2_s32){tx, ty}, ek_tile_type_dirt);
        }

        for (t_s32 ty = level + dirt_depth; ty < TILEMAP_HEIGHT; ty++) {
            AddTile(tm_core, (s_v2_s32){tx, ty}, ek_tile_type_stone);
        }

        if (RandPerc() < GROUND_LEVEL_OFFS_VAR) {
            if (RandPerc() < 0.5f) {
                level++;
            } else {
                level--;
            }

            level = CLAMP(level, GROUND_LEVEL_TOP, GROUND_LEVEL_BOTTOM - 1);
        }
    }
}

static void DesertGenStep(s_tilemap_core* const tm_core) {
    assert(tm_core);

    const bool desert_on_right = RandPerc() < 0.5f;
    const t_s32 desert_width = RandRangeS32(32, 48);
    const t_s32 desert_min_offs_from_mid = 40;
    const t_s32 desert_begin_tx = desert_on_right
        ? RandRangeS32((TILEMAP_WIDTH / 2) + desert_min_offs_from_mid, TILEMAP_WIDTH - desert_width)
        : RandRangeS32(0, (TILEMAP_WIDTH / 2) - desert_min_offs_from_mid - desert_width); // TODO: Not entirely correct?

    t_s32 level = GROUND_LEVEL_BASE + RandRangeS32(-GROUND_LEVEL_OFFS_LIM, GROUND_LEVEL_OFFS_LIM);

    for (t_s32 ty = 0; ty < TILEMAP_HEIGHT; ty++) {
        for (t_s32 tx = desert_begin_tx; tx < desert_begin_tx + desert_width; tx++) {
            if (!IsTileActive(&tm_core->activity, (s_v2_s32){tx, ty})) {
                continue;
            }

            e_tile_type* const tt = STATIC_ARRAY_2D_ELEM(tm_core->tile_types, ty, tx);

            if (*tt == ek_tile_type_grass || *tt == ek_tile_type_dirt) {
                *tt = ek_tile_type_sand;
            }
        }
    }
}

void GenWorld(s_world_core* const world_core) {
    assert(world_core && IS_ZERO(*world_core));

    world_core->player_hp_max = PLAYER_INIT_HP_MAX;

    GroundGenStep(&world_core->tilemap_core);
}
