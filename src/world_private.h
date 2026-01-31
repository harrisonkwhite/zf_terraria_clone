#pragma once

#include "world_public.h"
#include "sprites.h"


// ============================================================
// @section: Tiles
// ============================================================

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

constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

// @todo: Could possibly be made opaque?
struct t_tilemap {
    zcl::t_static_bitset<k_tilemap_size.x * k_tilemap_size.y> activity;
    zcl::t_static_array<zcl::t_static_array<t_tile_type_id, k_tilemap_size.x>, k_tilemap_size.y> types;
};

zcl::t_b8 TilePosCheckInBounds(const zcl::t_v2_i pos);

void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type);

void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos);

zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i tile_pos);

zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect);

zcl::t_b8 TileCollisionCheck(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void ProcessTileCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rendering_context, const t_assets *const assets);

// ============================================================


void WorldGen(zcl::t_rng *const rng, t_tilemap *const o_tilemap);
