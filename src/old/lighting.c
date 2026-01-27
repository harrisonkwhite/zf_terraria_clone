#include "lighting.h"

typedef struct {
    s_v2_s32_array positions;
    t_s32 cap;
    t_s32 start;
    t_s32 len;
} s_light_pos_queue;

static bool EnqueueLightPos(s_light_pos_queue* const queue, const s_v2_s32 light_pos, const s_v2_s32 map_size) {
    assert(IsLightPosInBounds(light_pos, map_size));

    if (queue->len < queue->cap) {
        *V2S32Elem(queue->positions, (queue->start + queue->len) % queue->cap) = light_pos;
        queue->len++;
        return true;
    }

    return false;
}

static s_v2_s32 DequeueLightPos(s_light_pos_queue* const queue, const s_v2_s32 map_size) {
    assert(queue->len > 0);

    const s_v2_s32 light = *V2S32Elem(queue->positions, queue->start);

    queue->start++;
    queue->start %= queue->cap;

    queue->len--;

    return light;
}

bool GenLightmap(s_lightmap* const lightmap, s_mem_arena* const mem_arena, const s_v2_s32 size) {
    assert(IS_ZERO(*lightmap));
    assert(size.x > 0 && size.y > 0);

    lightmap->levels = PushLightLevelArrayToMemArena(mem_arena, size.x * size.y);

    if (IS_ZERO(lightmap->levels)) {
        LOG_ERROR("Failed to allocate memory for lightmap!");
        return false;
    }

    lightmap->size = size;

    return true;
}

bool PropagateLights(const s_lightmap lightmap, s_mem_arena* const temp_mem_arena) {
    // Set up the light position queue.
    const t_s32 light_limit = lightmap.size.x * lightmap.size.y;

    s_light_pos_queue light_pos_queue = {
        .positions = PushV2S32ArrayToMemArena(temp_mem_arena, light_limit),
        .cap = light_limit
    };

    if (IS_ZERO(light_pos_queue.positions)) {
        LOG_ERROR("Failed to reserve memory for light positions!");
        return false;
    }

    // Enqueue lights already in the map.
    for (t_s32 y = 0; y < lightmap.size.y; y++) {
        for (t_s32 x = 0; x < lightmap.size.x; x++) {
            const s_v2_s32 pos = {x, y};
            const t_s32 i = IndexFrom2D(pos.x, pos.y, lightmap.size.x);

            const t_s32 lvl = LightLevel(lightmap, pos);
            assert(IsLightLevelValid(lvl));

            if (lvl > 0) {
                EnqueueLightPos(&light_pos_queue, pos, lightmap.size);
            }
        }
    }

    // Propagate lights (BFS).
    while (light_pos_queue.len > 0) {
        const s_v2_s32 pos = DequeueLightPos(&light_pos_queue, lightmap.size);

        const s_v2_s32 neighbour_pos_offsets[] = {
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0}
        };

        for (t_s32 i = 0; i < STATIC_ARRAY_LEN(neighbour_pos_offsets); i++) {
            const s_v2_s32 neighbour_pos = V2S32Sum(pos, *STATIC_ARRAY_ELEM(neighbour_pos_offsets, i));

            if (!IsLightPosInBounds(neighbour_pos, lightmap.size)) {
                continue;
            }

            const t_light_level new_neighbour_light_level = LightLevel(lightmap, pos) - 1;

            // Only brighten a neighbour, don't darken it.
            if (LightLevel(lightmap, neighbour_pos) < new_neighbour_light_level) {
                SetLightLevel(lightmap, neighbour_pos, new_neighbour_light_level);

                // No point in adding it to the queue it if it's completely dark, as there'd be no brightness to propagate.
                if (LightLevel(lightmap, neighbour_pos) > 0) {
                    EnqueueLightPos(&light_pos_queue, neighbour_pos, lightmap.size);
                }
            }
        }
    }

    return true;
}

void RenderLightmap(const s_rendering_context* const rendering_context, const s_lightmap map, const s_v2 pos, const t_r32 tile_size) {
    assert(tile_size > 0.0f);

    for (t_s32 ty = 0; ty < map.size.y; ty++) {
        for (t_s32 tx = 0; tx < map.size.x; tx++) {
            const t_light_level light_lvl = LightLevel(map, (s_v2_s32){tx, ty});

            if (light_lvl == LIGHT_LEVEL_LIMIT) {
                continue;
            }

            const s_rect rect = {
                pos.x + (tx * tile_size),
                pos.y + (ty * tile_size),
                tile_size,
                tile_size
            };

            const u_v4 blend = {
                .w = 1.0f - ((t_r32)light_lvl / LIGHT_LEVEL_LIMIT)
            };

            RenderRect(rendering_context, rect, blend);
        }
    }
}
