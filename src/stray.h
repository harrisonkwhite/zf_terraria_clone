#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_tilemap;

struct t_camera;

struct t_player_entity;

struct t_npc_manager;

struct t_pop_up_manager;

// ==================================================

zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void ProcessTilemapCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

zcl::t_b8 CheckOnGround(const zcl::t_rect_f collider, const t_tilemap *const tilemap);

zcl::t_rect_i CalcCameraTilemapRect(const t_camera *const camera, const t_tilemap *const tilemap, const zcl::t_v2_i screen_size);

void ProcessPlayerAndNPCCollisions(t_player_entity *const player_entity, const t_npc_manager *const npc_manager, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng, zcl::t_arena *const temp_arena);

// Returns true iff the tile position is in reach.
[[nodiscard]] zcl::t_b8 LoadHoveredTilePositionIfInReach(const zcl::t_v2 cursor_pos, const zcl::t_v2_i screen_size, const t_camera *const camera, const zcl::t_v2 player_pos, zcl::t_v2_i *const o_pos);
