#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

// ==================================================

struct t_cloud_manager;

t_cloud_manager *CloudsCreate(const zcl::t_v2 span, const zcl::t_array_rdonly<zcl::t_f32> layer_depths, zcl::t_rng *const rng, zcl::t_arena *const arena);

void CloudsUpdate(t_cloud_manager *const manager);

void CloudsRender(const t_cloud_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera);
