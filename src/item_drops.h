#pragma once

#include "items.h"

// ============================================================
// @section: External Forward Declarations

struct t_tilemap_core;

// ==================================================

struct t_item_drop_manager;

t_item_drop_manager *ItemDropManagerCreate(zcl::t_arena *const arena);

void ItemDropSpawn(t_item_drop_manager *const drop_manager, const zcl::t_v2 pos, const t_item_type_id item_type_id, const zcl::t_i32 item_quantity);

void ItemDropsProcessMovementAndCollection(t_item_drop_manager *const drop_manager, t_player_meta *const player_meta, const t_player_entity *const player_entity, const zcl::t_f32 gravity, const t_tilemap *const tilemap, t_pop_up_manager *const pop_up_manager, const zgl::t_audio_ticket_mut audio_ticket, const t_assets *const assets, zcl::t_rng *const rng, zcl::t_arena *const temp_arena);

void ItemDropsRender(const t_item_drop_manager *const drop_manager, const zgl::t_rendering_context rc, const t_assets *const assets);
