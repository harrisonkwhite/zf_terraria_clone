#pragma once

#if 0

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

// ==================================================

struct t_cloud_manager;

t_cloud_manager *CloudManagerCreate(const zcl::t_i32 cloud_cnt, zcl::t_arena *const arena);

void CloudManagerRenderAll(const t_cloud_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera);
#endif
