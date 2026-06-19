#pragma once

struct t_lightmap;

t_lightmap *LightmapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena);
void LightmapRender(const t_lightmap *const lightmap, const zgl::t_rendering_context rc);
