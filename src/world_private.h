#pragma once

#include "world_public.h"
#include "sprites.h"


// ============================================================
// @section: External Forward Declarations
// ============================================================

struct t_inventory;

// ============================================================


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

struct t_tilemap;

t_tilemap *TilemapCreate(zcl::t_arena *const arena);

zcl::t_b8 TilePosCheckInBounds(const zcl::t_v2_i pos);

void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i pos, const t_tile_type_id tile_type);
void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i pos);

zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i pos);

zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect);

zcl::t_b8 TileCollisionCheck(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void ProcessTileCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

// @todo: MAYBE this should be decided by the world, not the tiles submodule?
void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rendering_context, const t_assets *const assets);

// ============================================================


t_tilemap *WorldGen(zcl::t_rng *const rng, zcl::t_arena *const arena);

constexpr zcl::t_i32 k_player_inventory_slot_cnt = 28;


// ============================================================
// @section: UI
// ============================================================

struct t_world_ui;

t_world_ui *WorldUICreate(zcl::t_arena *const arena);
void WorldUITick(t_world_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state);
void WorldUIRender(const t_world_ui *const ui, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);

// ============================================================
