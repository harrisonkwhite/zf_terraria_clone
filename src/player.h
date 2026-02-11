#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

struct t_tilemap;

struct t_inventory;

struct t_pop_up_manager;

// ==================================================

struct t_player_meta;

struct t_player_entity;

t_player_meta *CreatePlayerMeta(zcl::t_arena *const arena);

t_player_entity *CreatePlayerEntity(const t_player_meta *const player_meta, const t_tilemap *const tilemap, zcl::t_arena *const arena);

void UpdatePlayerTimers(t_player_entity *const player_entity);

void UpdatePlayerMovement(t_player_entity *const player_entity, const zgl::t_input_state *const input_state, const zcl::t_f32 gravity, const t_tilemap *const tilemap);

void ProcessPlayerInventoryHotbarUpdates(t_player_meta *const player_meta, const zgl::t_input_state *const input_state);

void ProcessPlayerDeath(t_player_entity *const player_entity);

void ProcessPlayerItemUsage(t_player_entity *const player_entity, const zgl::t_input_state *const input_state, t_player_meta *const player_meta, t_camera *const camera, t_tilemap *const tilemap, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);

void HurtPlayer(t_player_entity *const player_entity, const zcl::t_i32 damage, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng);

void RenderPlayer(const zgl::t_rendering_context rc, const t_player_entity *const player_entity, const t_assets *const assets);

zcl::t_b8 CheckPlayerAlive(const t_player_entity *const player_entity);

zcl::t_i32 GetPlayerHealth(const t_player_entity *const player_entity);

zcl::t_i32 GetPlayerHealthLimit(const t_player_meta *const player_meta);

t_inventory *GetPlayerInventory(const t_player_meta *const player_meta);

zcl::t_i32 GetPlayerInventoryHotbarSlotSelectedIndex(const t_player_meta *const player_meta);

zcl::t_v2 GetPlayerPosition(const t_player_entity *const player_entity);

zcl::t_rect_f GetPlayerCollider(const zcl::t_v2 pos);
