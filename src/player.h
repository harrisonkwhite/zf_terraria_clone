#pragma once

#include "inventories.h"
#include "hitboxes.h"

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

struct t_tilemap;

struct t_item_drop_manager;

struct t_pop_up_manager;

// ==================================================

struct t_player_meta;

struct t_player_entity;

t_player_meta *PlayerMetaCreate(zcl::t_arena *const arena);

t_player_entity *PlayerEntityCreate(const t_player_meta *const player_meta, const t_tilemap *const tilemap, zcl::t_arena *const arena);

void PlayerEntityReset(t_player_entity *const player_entity, const t_player_meta *const player_meta, const t_tilemap *const tilemap);

void PlayerUpdateTimers(t_player_entity *const player_entity);

void PlayerUpdateMovement(t_player_entity *const player_entity, const zgl::t_input_state *const input_state, const zcl::t_f32 gravity, const t_tilemap *const tilemap);

void PlayerProcessInventoryHotbarUpdates(t_player_meta *const player_meta, const zgl::t_input_state *const input_state);

void PlayerProcessDeath(t_player_entity *const player_entity);

void PlayerProcessItemUsage(t_player_entity *const player_entity, const zgl::t_input_state *const input_state, t_player_meta *const player_meta, t_item_drop_manager *const item_drop_manager, t_camera *const camera, t_tilemap *const tilemap, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);

void PlayerProcessHitboxCollisions(t_player_entity *const player_entity, const zcl::t_array_rdonly<t_hitbox> hitboxes, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng);

void PlayerRender(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets);

zcl::t_b8 PlayerCheckAlive(const t_player_entity *const player_entity);

zcl::t_i32 PlayerGetHealth(const t_player_entity *const player_entity);

zcl::t_i32 PlayerGetHealthLimit(const t_player_meta *const player_meta);

t_inventory *PlayerGetInventory(const t_player_meta *const player_meta);

t_inventory_slot PlayerGetInventoryHotbarSlotSelected(const t_player_meta *const player_meta);

zcl::t_i32 PlayerGetInventoryHotbarSlotSelectedIndex(const t_player_meta *const player_meta);

zcl::t_v2 PlayerGetPosition(const t_player_entity *const player_entity);

zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 pos);
