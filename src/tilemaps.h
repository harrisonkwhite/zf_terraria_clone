#pragma once

#include "tiles.h"

// ============================================================
// @section: External Forward Declarations

struct t_camera;

// ==================================================

constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

struct t_tilemap;

t_tilemap *TilemapCreate(zcl::t_arena *const arena);

zcl::t_b8 TilemapCheckTilePosInBounds(const zcl::t_v2_i pos);

// The tile position MUST be empty.
void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type);

// The tile position MUST NOT be empty.
void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos);

void TilemapHurt(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage);

// Is the tile at the given position empty?
zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i tile_pos);

zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect);

zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

zcl::t_v2 TilemapMoveContact(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void TilemapProcessCollisions(const t_tilemap *const tilemap, zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin);

void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets);

zcl::t_v2_i ScreenToTilePos(const zcl::t_v2 pos_screen, const zcl::t_v2_i screen_size, const t_camera *const camera);
