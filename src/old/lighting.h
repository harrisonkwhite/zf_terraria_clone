#ifndef LIGHTING_H
#define LIGHTING_H

#include <zfwc.h>

#define LIGHT_LEVEL_LIMIT (t_light_level)12

typedef t_s8 t_light_level;

DEF_ARRAY_TYPE(t_light_level, light_level, LightLevel);

typedef struct {
    s_light_level_array levels;
    s_v2_s32 size;
} s_lightmap;

bool WARN_UNUSED_RESULT GenLightmap(s_lightmap* const lightmap, s_mem_arena* const mem_arena, const s_v2_s32 size);
bool WARN_UNUSED_RESULT PropagateLights(const s_lightmap lightmap, s_mem_arena* const temp_mem_arena);
void RenderLightmap(const s_rendering_context* const rendering_context, const s_lightmap map, const s_v2 pos, const t_r32 tile_size);

static inline bool IsLightPosInBounds(const s_v2_s32 pos, const s_v2_s32 map_size) {
    return pos.x >= 0 && pos.x < map_size.x && pos.y >= 0 && pos.y < map_size.y;
}

static inline t_light_level LightLevel(const s_lightmap map, const s_v2_s32 pos) {
    assert(IsLightPosInBounds(pos, map.size));

    const t_s32 index = IndexFrom2D(pos.x, pos.y, map.size.x);
    return *LightLevelElem(map.levels, index);
}

static inline bool IsLightLevelValid(const t_light_level lvl) {
    return lvl >= 0 && lvl <= LIGHT_LEVEL_LIMIT;
}

static inline void SetLightLevel(const s_lightmap map, const s_v2_s32 pos, const t_light_level lvl) {
    assert(IsLightPosInBounds(pos, map.size));
    assert(IsLightLevelValid(lvl));

    const t_s32 index = IndexFrom2D(pos.x, pos.y, map.size.x);
    *LightLevelElem(map.levels, index) = lvl;
}

#endif
