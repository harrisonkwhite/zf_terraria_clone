#pragma once

#include "sprites.h"

enum t_item_type_id : zcl::t_i32 {
    ek_item_type_id_dirt_block,
    ek_item_type_id_stone_block,
    ek_item_type_id_grass_block,

    ekm_item_type_id_cnt
};

// Core data relevant to the item type in potentially any context. For example, in the title screen there could be menu listing all the item types and their icons.
struct t_item_type_basic_info {
    zcl::t_str_rdonly name;
    t_sprite_id icon_sprite_id;
    zcl::t_i32 quantity_limit;
};

constexpr zcl::t_i32 k_item_type_default_block_use_time = 0;
constexpr zcl::t_i32 k_item_type_default_block_stack_limit = 99;

inline const zcl::t_static_array<t_item_type_basic_info, ekm_item_type_id_cnt> g_item_type_infos_basic = {{
    {
        .name = ZCL_STR_LITERAL("Dirt Block"),
        .icon_sprite_id = ek_sprite_id_dirt_block_item_icon,
        .quantity_limit = k_item_type_default_block_stack_limit,
    },
    {
        .name = ZCL_STR_LITERAL("Stone Block"),
        .icon_sprite_id = ek_sprite_id_stone_block_item_icon,
        .quantity_limit = k_item_type_default_block_stack_limit,
    },
    {
        .name = ZCL_STR_LITERAL("Grass Block"),
        .icon_sprite_id = ek_sprite_id_grass_block_item_icon,
        .quantity_limit = k_item_type_default_block_stack_limit,
    },
}};
