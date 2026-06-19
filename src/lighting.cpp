#include "lighting.h"

using t_light_level = zcl::t_i8;

constexpr t_light_level k_light_level_limit = 12;

struct t_lightmap {
    zcl::t_v2_i size;
    zcl::t_array_mut<t_light_level> lvls;
};

t_lightmap *LightmapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_lightmap>(arena);
    result->size = size;
    result->lvls = zcl::ArenaPushArray<t_light_level>(arena, size.x * size.y);

    return result;
}

void LightmapRender(const t_lightmap *const lightmap, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const zcl::t_f32 tile_size) {
    for (zcl::t_i32 ty = 0; ty < lightmap->size.y; ty++) {
        for (zcl::t_i32 tx = 0; tx < lightmap->size.x; tx++) {
            const auto light_lvl = lightmap->lvls[(ty * lightmap->size.y) + tx];

            if (light_lvl == k_light_level_limit) {
                continue;
            }

            const auto rect = zcl::RectCreateF(pos + zcl::t_v2{tx * tile_size, ty * tile_size}, {tile_size, tile_size});

            const auto color = zcl::t_color_rgba32f{
                .a = 1.0f - (static_cast<zcl::t_f32>(light_lvl) / k_light_level_limit),
            };

            zgl::RendererSubmitRect(rc, rect, color);
        }
    }
}
