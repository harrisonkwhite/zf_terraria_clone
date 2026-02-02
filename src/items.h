#pragma once

#include "sprites.h"

enum t_item_type_id : zcl::t_i32 {
    ek_item_type_id_dirt_block,
    ek_item_type_id_stone_block,
    ek_item_type_id_grass_block,

    ekm_item_type_id_cnt
};

struct t_item_type {
    zcl::t_str_rdonly name;
    t_sprite_id icon_sprite_id;
    zcl::t_i32 use_time;
    // @todo: Flags? e.g. show_tile_highlight
};

constexpr zcl::t_i32 k_item_type_default_block_use_time = 0;

inline const zcl::t_static_array<t_item_type, ekm_item_type_id_cnt> g_item_types = {{
    {
        .name = ZCL_STR_LITERAL("Dirt Block"),
        .icon_sprite_id = ek_sprite_id_dirt_block_item_icon,
        .use_time = k_item_type_default_block_use_time,
    },
    {
        .name = ZCL_STR_LITERAL("Stone Block"),
        .icon_sprite_id = ek_sprite_id_stone_block_item_icon,
        .use_time = k_item_type_default_block_use_time,
    },
    {
        .name = ZCL_STR_LITERAL("Grass Block"),
        .icon_sprite_id = ek_sprite_id_grass_block_item_icon,
        .use_time = k_item_type_default_block_use_time,
    },
}};
