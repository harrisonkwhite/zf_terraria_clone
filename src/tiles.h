#pragma once

#include "sprites.h"

struct t_tile_type {
    t_sprite_id sprite;
};

enum t_tile_type_id : zcl::t_i8 {
    ek_tile_type_id_dirt,
    ek_tile_type_id_stone,
    ek_tile_type_id_grass,

    ekm_tile_type_id_cnt
};

constexpr zcl::t_static_array<t_tile_type, ekm_tile_type_id_cnt> k_tile_types = {{
    {.sprite = ek_sprite_id_dirt_tile},
    {.sprite = ek_sprite_id_stone_tile},
    {.sprite = ek_sprite_id_grass_tile},
}};

constexpr zcl::t_i32 k_tile_size = 8;

static_assert(
    []() {
        for (zcl::t_i32 i = 0; i < ekm_tile_type_id_cnt; i++) {
            if (zcl::RectGetSize(k_sprites[k_tile_types[i].sprite].src_rect) != zcl::t_v2_i{k_tile_size, k_tile_size}) {
                return false;
            }
        }

        return true;
    }(),
    "Tile size must be consistent with sprite sizes!");

constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

struct t_tilemap;

t_tilemap *TilemapCreate(zcl::t_arena *const arena);

zcl::t_b8 TilemapPosCheckInBounds(const zcl::t_v2_i pos);

void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i pos, const t_tile_type_id tile_type);
void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i pos);

zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i pos);

zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect);

zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets);
