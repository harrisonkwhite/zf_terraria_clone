#pragma once

#include "sprites.h"
#include "items.h"

struct t_tile_type {
    t_sprite_id sprite;
    zcl::t_i32 health;
    t_item_type_id drop_item_type_id;
};

enum t_tile_type_id : zcl::t_i8 {
    ek_tile_type_id_dirt,
    ek_tile_type_id_stone,
    ek_tile_type_id_grass,

    ekm_tile_type_id_cnt
};

constexpr zcl::t_static_array<t_tile_type, ekm_tile_type_id_cnt> k_tile_types = {{
    {.sprite = ek_sprite_id_tile_dirt, .health = 60},
    {.sprite = ek_sprite_id_tile_stone, .health = 90},
    {.sprite = ek_sprite_id_tile_grass, .health = 60},
}};

constexpr zcl::t_i32 k_tile_size = 8;

constexpr zcl::t_b8 TileTypesCheckValid() {
    for (zcl::t_i32 i = 0; i < ekm_tile_type_id_cnt; i++) {
        if (zcl::RectGetSize(k_sprites[k_tile_types[i].sprite].src_rect) != zcl::t_v2_i{k_tile_size, k_tile_size}) {
            return false;
        }

        if (k_tile_types[i].health <= 0) {
            return false;
        }
    }

    return true;
}

static_assert(TileTypesCheckValid(), "One or more tile types are invalid!");

constexpr zcl::t_u8 k_tile_life_limit = 240;

struct t_tilemap_core;

t_tilemap_core *TilemapCoreCreate(const zcl::t_v2_i size, zcl::t_arena *const arena);

void TilemapCoreAdd(t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type_id);

void TilemapCoreRemove(t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos);

struct t_tilemap;

t_tilemap *TilemapCreate(t_tilemap_core *const core, const zcl::t_v2_i chunk_size, zcl::t_arena *const arena);

void TilemapPlace(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type_id);

void TilemapHurt(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage);

void TilemapRender(const t_tilemap *const tilemap, const zgl::t_rendering_context rc, const zcl::t_rect_i tilemap_subset, const t_assets *const assets);

t_tilemap_core *TilemapGetCore(t_tilemap *const tilemap);

const t_tilemap_core *TilemapGetCore(const t_tilemap *const tilemap);

zcl::t_b8 TilemapCheck(const t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos);

inline zcl::t_b8 TilemapCheck(const t_tilemap *const tilemap, const zcl::t_v2_i tile_pos) {
    return TilemapCheck(TilemapGetCore(tilemap), tile_pos);
}

zcl::t_v2_i TilemapGetSize(const t_tilemap_core *const tilemap_core);

inline zcl::t_v2_i TilemapGetSize(const t_tilemap *const tilemap) {
    return TilemapGetSize(TilemapGetCore(tilemap));
}

zcl::t_b8 TilemapCheckTilePosInBounds(const t_tilemap_core *const tilemap_core, const zcl::t_v2_i tile_pos);

inline zcl::t_b8 TilemapCheckTilePosInBounds(const t_tilemap *const tilemap, const zcl::t_v2_i tile_pos) {
    return TilemapCheckTilePosInBounds(TilemapGetCore(tilemap), tile_pos);
}

zcl::t_rect_i TilemapCalcRectSpan(const t_tilemap_core *const tilemap_core, const zcl::t_rect_f rect);

inline zcl::t_rect_i TilemapCalcRectSpan(const t_tilemap *const tilemap, const zcl::t_rect_f rect) {
    return TilemapCalcRectSpan(TilemapGetCore(tilemap), rect);
}

zcl::t_b8 TilemapCheckCollision(const t_tilemap_core *const tilemap_core, const zcl::t_rect_f collider);

inline zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider) {
    return TilemapCheckCollision(TilemapGetCore(tilemap), collider);
}
