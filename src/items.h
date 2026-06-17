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

struct t_hitbox_manager;

struct t_pop_up_manager;

// ==================================================

constexpr zcl::t_f32 k_item_tile_reach_dist = 5.0f;

enum t_item_type_id : zcl::t_i32 {
    ek_item_type_id_dirt_block,
    ek_item_type_id_stone_block,
    ek_item_type_id_grass_block,
    ek_item_type_id_copper_pickaxe,
    ek_item_type_id_copper_sword,

    ekm_item_type_id_cnt
};

enum t_item_type_flags : zcl::t_u8 {
    ek_item_type_flags_none = 0,

    ek_item_type_flag_use_consume = 1 << 0, // Does the item get removed from inventory on use?
    ek_item_type_flag_use_hold = 1 << 1,    // Can you repeat item use by holding the button down?
    ek_item_type_flag_show_tile_highlight = 1 << 2,
};

constexpr t_item_type_flags operator|(const t_item_type_flags a, const t_item_type_flags b) {
    return static_cast<t_item_type_flags>(static_cast<zcl::t_u8>(a) | static_cast<zcl::t_u8>(b));
}

constexpr t_item_type_flags operator&(const t_item_type_flags a, const t_item_type_flags b) {
    return static_cast<t_item_type_flags>(static_cast<zcl::t_u8>(a) & static_cast<zcl::t_u8>(b));
}

struct t_item_type {
    zcl::t_str_rdonly name;
    t_sprite_id sprite_id;
    zcl::t_v2 origin;
    zcl::t_i32 quantity_limit;
    zcl::t_i32 use_time; // The minimum length of the break in ticks between each item use.
    t_item_type_flags flags;
};

constexpr zcl::t_i32 k_item_type_default_block_quantity_limit = 99;
constexpr zcl::t_i32 k_item_type_default_block_use_time = 0;
constexpr t_item_type_flags k_item_type_default_block_flags = ek_item_type_flag_use_consume | ek_item_type_flag_use_hold | ek_item_type_flag_show_tile_highlight;

inline const zcl::t_static_array<t_item_type, ekm_item_type_id_cnt> g_item_types = {{
    {
        .name = ZCL_STR_LITERAL("Dirt Block"),
        .sprite_id = ek_sprite_id_item_dirt_block,
        .origin = zcl::k_origin_center,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .flags = k_item_type_default_block_flags,
    },
    {
        .name = ZCL_STR_LITERAL("Stone Block"),
        .sprite_id = ek_sprite_id_item_stone_block,
        .origin = zcl::k_origin_center,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .flags = k_item_type_default_block_flags,
    },
    {
        .name = ZCL_STR_LITERAL("Grass Block"),
        .sprite_id = ek_sprite_id_item_grass_block,
        .origin = zcl::k_origin_center,
        .quantity_limit = k_item_type_default_block_quantity_limit,
        .use_time = k_item_type_default_block_use_time,
        .flags = k_item_type_default_block_flags,
    },
    {
        .name = ZCL_STR_LITERAL("Copper Pickaxe"),
        .sprite_id = ek_sprite_id_item_copper_pickaxe,
        .origin = zcl::k_origin_center_left,
        .quantity_limit = 1,
        .use_time = 15,
        .flags = ek_item_type_flag_use_hold | ek_item_type_flag_show_tile_highlight,
    },
    {
        .name = ZCL_STR_LITERAL("Copper Sword"),
        .sprite_id = ek_sprite_id_item_copper_sword,
        .origin = zcl::k_origin_center_left,
        .quantity_limit = 1,
        .use_time = 20,
        .flags = ek_item_type_flag_use_hold,
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

    t_item_drop_manager *item_drop_manager;

    t_hitbox_manager *hitbox_manager;
};

using t_item_type_use_func = zcl::t_b8 (*)(const t_item_type_use_func_context &context);

extern const zcl::t_static_array<t_item_type_use_func, ekm_item_type_id_cnt> g_item_type_use_funcs;
