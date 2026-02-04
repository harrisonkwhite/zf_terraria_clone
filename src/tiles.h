#pragma once

#include "sprites.h"
#include "items.h"

struct t_tile_type {
    t_sprite_id sprite;
    zcl::t_u8 life;
    t_item_type_id drop_item_type_id;
};

enum t_tile_type_id : zcl::t_i8 {
    ek_tile_type_id_dirt,
    ek_tile_type_id_stone,
    ek_tile_type_id_grass,

    ekm_tile_type_id_cnt
};

constexpr zcl::t_static_array<t_tile_type, ekm_tile_type_id_cnt> k_tile_types = {{
    {.sprite = ek_sprite_id_tile_dirt, .life = 60},
    {.sprite = ek_sprite_id_tile_stone, .life = 90},
    {.sprite = ek_sprite_id_tile_grass, .life = 60},
}};

constexpr zcl::t_i32 k_tile_size = 8;

constexpr zcl::t_b8 TileTypeSpriteSizesCheckValid() {
    for (zcl::t_i32 i = 0; i < ekm_tile_type_id_cnt; i++) {
        if (zcl::RectGetSize(k_sprites[k_tile_types[i].sprite].src_rect) != zcl::t_v2_i{k_tile_size, k_tile_size}) {
            return false;
        }
    }

    return true;
}

static_assert(TileTypeSpriteSizesCheckValid(), "Tile size must be consistent with sprite sizes!");

constexpr zcl::t_u8 k_tile_life_limit = 240;
