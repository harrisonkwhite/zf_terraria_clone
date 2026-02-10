#pragma once

#include "tiles.h"

struct t_tilemap;

t_tilemap *TilemapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena);

zcl::t_b8 TilemapCheck(const t_tilemap *const tilemap, const zcl::t_v2_i tile_pos);

void TilemapAdd(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type);

void TilemapRemove(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type);

zcl::t_b8 TilemapCheckTilePosInBounds(const zcl::t_v2_i tilemap_size, const zcl::t_v2_i tile_pos);
