#include "lighting.h"

using t_light_level = zcl::t_i8;

struct t_lightmap {
    zcl::t_array_mut<t_light_level> lvls;
};

t_lightmap *LightmapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_lightmap>(arena);
    result->lvls = zcl::ArenaPushArray<t_light_level>(arena, size.x * size.y);

    return result;
}

void LightmapRender(const t_lightmap *const lightmap, const zgl::t_rendering_context rc) {
}
