#pragma once

using t_light_level = zcl::t_i8;

constexpr t_light_level k_light_level_limit = 12;

struct t_lightmap;

t_lightmap *LightmapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena);
void LightmapSetLevel(t_lightmap *const lightmap, const zcl::t_v2_i pos, const t_light_level lvl);
void LightmapPropagate(const t_lightmap *const lightmap, zcl::t_arena *const temp_arena);
void LightmapRender(const t_lightmap *const lightmap, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const zcl::t_f32 tile_size);
