#pragma once

#include "items.h"

// ============================================================
// @section: External Forward Declarations

struct t_tilemap_core;

// ==================================================

// @todo: Encapsulate.
constexpr zcl::t_i32 k_item_drop_limit = 1024;

struct t_item_drop {
    zcl::t_v2 pos;
    zcl::t_v2 vel;

    t_item_type_id item_type_id;
    zcl::t_i32 item_quantity;
};

struct t_item_drop_manager {
    zcl::t_static_array<t_item_drop, k_item_drop_limit> buf;
    zcl::t_static_bitset<k_item_drop_limit> activity;
};

void ItemDropSpawn(t_item_drop_manager *const drop_manager, const zcl::t_v2 pos, const t_item_type_id item_type_id, const zcl::t_i32 item_quantity);

void ItemDropsProcessMovementAndCollection(t_item_drop_manager *const drop_manager, t_player_meta *const player_meta, const t_player_entity *const player_entity, const zcl::t_f32 gravity, const t_tilemap *const tilemap, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng);

void ItemDropsRender(const t_item_drop_manager *const drop_manager, const zgl::t_rendering_context rc, const t_assets *const assets);
