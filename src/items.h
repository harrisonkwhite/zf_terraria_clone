#pragma once

#include "sprites.h"

enum t_item_type_id : zcl::t_i32 {
    ek_item_type_id_copper_pickaxe,
    ek_item_type_id_dirt_block,
    ek_item_type_id_stone_block,
    ek_item_type_id_grass_block,

    ekm_item_type_id_cnt
};

struct t_item_type {
    zcl::t_str_rdonly name;
    t_sprite_id icon_sprite_id;
    zcl::t_i32 quantity_limit;
    zcl::t_i32 use_time;   // The length of the break in ticks between each item use.
    zcl::t_b8 use_consume; // Does the item get removed from inventory on use?
};

constexpr zcl::t_i32 k_item_type_default_block_use_time = 0;
constexpr zcl::t_i32 k_item_type_default_block_quantity_limit = 99;

inline const zcl::t_static_array<t_item_type, ekm_item_type_id_cnt> g_item_types = {{
    {
        .name = ZCL_STR_LITERAL("Copper Pickaxe"),
        .icon_sprite_id = ek_sprite_id_stone_block_item_icon, // @temp
        .quantity_limit = 1,
        .use_time = 10,
        .use_consume = false,
    },
    {
        .name = ZCL_STR_LITERAL("Dirt Block"),
        .icon_sprite_id = ek_sprite_id_dirt_block_item_icon,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
    },
    {
        .name = ZCL_STR_LITERAL("Stone Block"),
        .icon_sprite_id = ek_sprite_id_stone_block_item_icon,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
    },
    {
        .name = ZCL_STR_LITERAL("Grass Block"),
        .icon_sprite_id = ek_sprite_id_grass_block_item_icon,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
    },
}};
