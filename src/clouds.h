#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

// ==================================================

struct t_cloud_layer;

t_cloud_layer *CloudLayerCreate(const zcl::t_v2 span, const zcl::t_f32 parallax, const zcl::t_f32 scale, const zcl::t_f32 alpha, zcl::t_rng *const rng, zcl::t_arena *const arena);

void CloudLayerUpdate(t_cloud_layer *const layer);

void CloudLayerRender(const t_cloud_layer *const layer, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera);
