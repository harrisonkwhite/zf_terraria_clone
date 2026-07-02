#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_camera;

// ==================================================

struct t_sky;

struct t_sky_layer_info {
    zcl::t_f32 parallax;
    zcl::t_v2_i cloud_grid_dims;
    zcl::t_f32 cloud_chance;
};

t_sky *SkyCreate(const zcl::t_array_rdonly<t_sky_layer_info> layer_infos, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_rng *const rng, zcl::t_arena *const arena);

void SkyUpdate(t_sky *const sky, const zgl::t_gfx_ticket_rdonly gfx_ticket, const t_camera *const camera, const zcl::t_v2_i screen_size, const t_assets *const assets);

void SkyRender(const t_sky *const sky, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera);
