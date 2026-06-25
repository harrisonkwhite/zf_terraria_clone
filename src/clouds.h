#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

// ==================================================

struct t_cloud_layer;

t_cloud_layer *CloudLayerCreate(const zcl::t_f32 parallax, const t_camera *const camera, const zcl::t_v2_i screen_size, const zcl::t_v2_i world_size, zcl::t_rng *const rng, zcl::t_arena *const arena);

void CloudLayerUpdate(t_cloud_layer *const layer, const zgl::t_gfx_ticket_rdonly gfx_ticket, const t_assets *const assets);

void CloudLayerRender(const t_cloud_layer *const layer, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera);
