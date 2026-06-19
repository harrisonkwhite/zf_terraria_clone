#include "lighting.h"

#define DO_LIGHT_POS_QUEUE_VALIDITY_CHECKS false // Having this enabled slows things down tremendously. Only have active if needing to make sure the queue never enters an invalid state.

// NOTE: Using a queue system instead of recursion as that would blow up the stack.
typedef struct {
    zfw_s_vec_2d_s32* buf;
    int cap;
    int start;
    int len;
} s_light_pos_queue;

static bool IsLightPosQueueValid(const s_light_pos_queue* const queue, const zfw_s_vec_2d_s32 map_size) {
    if (!queue->buf
        || queue->cap <= 0
        || queue->start < 0 || queue->start >= queue->cap
        || queue->len < 0 || queue->len > queue->cap) {
        return false;
    }

    for (int i = 0; i < queue->len; i++) {
        const int bi = (queue->start + i) % queue->cap;
        const zfw_s_vec_2d_s32 pos = queue->buf[bi];

        if (!IsLightPosInBounds(pos, map_size)) {
            return false;
        }
    }

    return true;
}

static bool EnqueueLightPos(s_light_pos_queue* const queue, const zfw_s_vec_2d_s32 light_pos, const zfw_s_vec_2d_s32 map_size) {
#if DO_LIGHT_POS_QUEUE_VALIDITY_CHECKS
    assert(IsLightPosQueueValid(queue, map_size));
#endif
    assert(IsLightPosInBounds(light_pos, map_size));

    if (queue->len < queue->cap) {
        queue->buf[(queue->start + queue->len) % queue->cap] = light_pos;
        queue->len++;
        return true;
    }

    return false;
}

static zfw_s_vec_2d_s32 DequeueLightPos(s_light_pos_queue* const queue, const zfw_s_vec_2d_s32 map_size) {
#if DO_LIGHT_POS_QUEUE_VALIDITY_CHECKS
    assert(IsLightPosQueueValid(queue, map_size));
#endif
    assert(queue->len > 0);

    const zfw_s_vec_2d_s32 light = queue->buf[queue->start];

    queue->start++;
    queue->start %= queue->cap;

    queue->len--;

    return light;
}

s_lightmap GenLightmap(s_mem_arena* const mem_arena, const zfw_s_vec_2d_s32 size) {
    assert(size.x > 0 && size.y > 0);

    t_light_level* const buf = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, t_light_level, size.x * size.y);

    if (!buf) {
        LOG_ERROR("Failed to allocate memory for lightmap!");
        return (s_lightmap){0};
    }

    return (s_lightmap){
        .buf = buf,
        .size = size
    };
}

bool PropagateLights(const s_lightmap* const lightmap, s_mem_arena* const temp_mem_arena) {
    assert(IsLightmapInitted(lightmap));

    // Set up the light position queue.
    const int light_limit = lightmap->size.x * lightmap->size.y;

    s_light_pos_queue light_pos_queue = {
        .buf = MEM_ARENA_PUSH_TYPE_CNT(temp_mem_arena, zfw_s_vec_2d_s32, light_limit),
        .cap = light_limit
    };

    if (!light_pos_queue.buf) {
        LOG_ERROR("Failed to allocate memory for light position queue!");
        return false;
    }

    // Enqueue lights already in the map.
    for (int y = 0; y < lightmap->size.y; y++) {
        for (int x = 0; x < lightmap->size.x; x++) {
            const zfw_s_vec_2d_s32 pos = {x, y};
            const int i = IndexFrom2D(pos.x, pos.y, lightmap->size.x);

            const int lvl = LightLevel(lightmap, pos);
            assert(lvl >= 0 && lvl <= LIGHT_LEVEL_LIMIT);

            if (lvl > 0) {
                EnqueueLightPos(&light_pos_queue, pos, lightmap->size);
            }
        }
    }

    // Propagate lights (BFS).
    while (light_pos_queue.len > 0) {
        const zfw_s_vec_2d_s32 pos = DequeueLightPos(&light_pos_queue, lightmap->size);

        const zfw_s_vec_2d_s32 neighbour_pos_offsets[] = {
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0},
        };

        for (int i = 0; i < STATIC_ARRAY_LEN(neighbour_pos_offsets); i++) {
            const zfw_s_vec_2d_s32 neighbour_pos = ZFW_Vec2DS32Sum(pos, neighbour_pos_offsets[i]);

            if (!IsLightPosInBounds(neighbour_pos, lightmap->size)) {
                continue;
            }

            const t_light_level new_neighbour_light_level = LightLevel(lightmap, pos) - 1;

            // Only brighten a neighbour, don't darken it.
            if (LightLevel(lightmap, neighbour_pos) < new_neighbour_light_level) {
                SetLightLevel(lightmap, neighbour_pos, new_neighbour_light_level);

                // No point in adding it to the queue it if it's completely dark, as there'd be no brightness to propagate.
                if (LightLevel(lightmap, neighbour_pos) > 0) {
                    EnqueueLightPos(&light_pos_queue, neighbour_pos, lightmap->size);
                }
            }
        }
    }

    return true;
}

void RenderLightmap(const zfw_s_rendering_context* const rendering_context, const s_lightmap* const map, const zfw_s_vec_2d pos, const float tile_size) {
    assert(IsLightmapInitted(map));
    assert(tile_size > 0.0f);

    for (int ty = 0; ty < map->size.y; ty++) {
        for (int tx = 0; tx < map->size.x; tx++) {
            const t_light_level light_lvl = LightLevel(map, (zfw_s_vec_2d_s32){tx, ty});

            if (light_lvl == LIGHT_LEVEL_LIMIT) {
                continue;
            }

            const zfw_s_rect rect = {
                pos.x + (tx * tile_size),
                pos.y + (ty * tile_size),
                tile_size,
                tile_size
            };

            const zfw_u_vec_4d blend = {
                .w = 1.0f - ((float)light_lvl / LIGHT_LEVEL_LIMIT)
            };

            ZFW_RenderRect(rendering_context, rect, blend);
        }
    }
}
