#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_tilemap;

struct t_pop_up_manager;

// ==================================================

struct t_player_meta;

struct t_player_entity;

t_player_meta CreatePlayerMeta(zcl::t_arena *const arena);

zcl::t_v2 GetPlayerColliderSize();

t_player_entity CreatePlayerEntity(const t_player_meta *const player_meta, const t_tilemap *const tilemap);

zcl::t_rect_f GetPlayerCollider(const zcl::t_v2 pos);

zcl::t_b8 CheckPlayerGrounded(const zcl::t_v2 player_entity_pos, const t_tilemap *const tilemap);

void UpdatePlayerTimers(t_player_entity *const player_entity);

void PlayerUpdateMovement(t_player_entity *const player_entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state);

void ProcessPlayerInventoryHotbarUpdates(t_player_meta *const player_meta, const zgl::t_input_state *const input_state);

void ProcessPlayerDeath(t_player_meta *const player_meta, t_player_entity *const player_entity);

void HurtPlayer(t_player_entity *const player_entity, const zcl::t_i32 damage, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng);

void RenderPlayer(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets);
