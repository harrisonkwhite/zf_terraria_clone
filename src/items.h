#pragma once

#include "sprites.h"

// ============================================================
// @section: External Forward Declarations

struct t_tilemap;

struct t_camera;

struct t_player_meta;

struct t_player_entity;

struct t_npc_manager;

struct t_item_drop_manager;

struct t_pop_up_manager;

// ==================================================

enum t_item_type_id : zcl::t_i32 {
    ek_item_type_id_dirt_block,
    ek_item_type_id_stone_block,
    ek_item_type_id_grass_block,
    ek_item_type_id_copper_pickaxe,

    ekm_item_type_id_cnt
};

struct t_item_type {
    zcl::t_str_rdonly name;
    t_sprite_id icon_sprite_id;
    zcl::t_i32 quantity_limit;
    zcl::t_i32 use_time;   // The minimum length of the break in ticks between each item use.
    zcl::t_b8 use_consume; // Does the item get removed from inventory on use?
    zcl::t_b8 use_hold;    // Can you repeat item use by holding the button down?
};

constexpr zcl::t_i32 k_item_type_default_block_use_time = 0;
constexpr zcl::t_i32 k_item_type_default_block_quantity_limit = 99;

inline const zcl::t_static_array<t_item_type, ekm_item_type_id_cnt> g_item_types = {{
    {
        .name = ZCL_STR_LITERAL("Dirt Block"),
        .icon_sprite_id = ek_sprite_id_item_icon_dirt_block,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_hold = true,
    },
    {
        .name = ZCL_STR_LITERAL("Stone Block"),
        .icon_sprite_id = ek_sprite_id_item_icon_stone_block,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_hold = true,
    },
    {
        .name = ZCL_STR_LITERAL("Grass Block"),
        .icon_sprite_id = ek_sprite_id_item_icon_grass_block,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_hold = true,
    },
    {
        .name = ZCL_STR_LITERAL("Copper Pickaxe"),
        .icon_sprite_id = ek_sprite_id_item_icon_copper_pickaxe,
        .quantity_limit = 1,
        .use_time = 10,
        .use_consume = false,
        .use_hold = true,
    },
}};

struct t_item_type_use_func_context {
    zcl::t_arena *temp_arena;

    zcl::t_v2 cursor_pos;
    zcl::t_v2_i screen_size;

    t_camera *camera;

    t_tilemap *tilemap;

    t_player_meta *player_meta;
    t_player_entity *player_entity;
};

using t_item_type_use_func = zcl::t_b8 (*)(const t_item_type_use_func_context &context);

extern const zcl::t_static_array<t_item_type_use_func, ekm_item_type_id_cnt> g_item_type_use_funcs;
