#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_tilemap;

struct t_camera;

// ==================================================

zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void ProcessTilemapCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

zcl::t_rect_i CalcCameraTilemapRect(const t_camera *const camera, const t_tilemap *const tilemap, const zcl::t_v2_i screen_size);
