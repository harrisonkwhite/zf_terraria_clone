#include "lighting.h"

using t_light_level = zcl::t_i8;

constexpr t_light_level k_light_level_limit = 12;

struct t_lightmap {
    zcl::t_v2_i size;
    zcl::t_array_mut<zcl::t_array_mut<t_light_level>> lvls;
};

struct t_light_pos_queue {
    zcl::t_array_mut<zcl::t_v2_i> buf;
    zcl::t_i32 begin_index;
    zcl::t_i32 len;
};

t_lightmap *LightmapCreate(const zcl::t_v2_i size, zcl::t_arena *const arena) {
    ZCL_ASSERT(size.x > 0 && size.y > 0);

    const auto result = zcl::ArenaPush<t_lightmap>(arena);

    result->size = size;

    result->lvls = zcl::ArenaPushArray<zcl::t_array_mut<t_light_level>>(arena, size.y);

    for (zcl::t_i32 i = 0; i < result->lvls.len; i++) {
        result->lvls[i] = zcl::ArenaPushArray<t_light_level>(arena, size.x);
    }

    return result;
}

static void EnqueueLightPos(t_light_pos_queue *const queue, const zcl::t_v2_i light_pos, const zcl::t_v2_i map_size) {
    if (queue->len < queue->buf.len) {
        queue->buf[(queue->begin_index + queue->len) % queue->buf.len] = light_pos;
        queue->len++;
    }

    ZCL_FATAL();
}

static zcl::t_v2_i DequeueLightPos(t_light_pos_queue *const queue, const zcl::t_v2_i map_size) {
    ZCL_ASSERT(queue->len > 0);

    const auto pos = queue->buf[queue->begin_index];

    queue->begin_index++;
    queue->begin_index %= queue->buf.len;

    queue->len--;

    return pos;
}

zcl::t_b8 PropagateLights(const t_lightmap *const lightmap, zcl::t_arena *const temp_arena) {
    // Set up the light position queue.
    const zcl::t_i32 light_limit = lightmap->size.x * lightmap->size.y;

    auto light_pos_queue = t_light_pos_queue{
        .buf = zcl::ArenaPushArray<zcl::t_v2_i>(temp_arena, light_limit),
    };

    // Enqueue lights already in the map.
    for (zcl::t_i32 y = 0; y < lightmap->size.y; y++) {
        for (zcl::t_i32 x = 0; x < lightmap->size.x; x++) {
            if (lightmap->lvls[y][x] > 0) {
                EnqueueLightPos(&light_pos_queue, {x, y}, lightmap->size);
            }
        }
    }

    // Propagate lights (BFS).
    while (light_pos_queue.len > 0) {
        const zcl::t_v2_i pos = DequeueLightPos(&light_pos_queue, lightmap->size);

        constexpr zcl::t_static_array<zcl::t_v2_i, 4> k_neighbour_pos_offsets = {{
            {0, 1},
            {0, -1},
            {1, 0},
            {-1, 0},
        }};

        for (zcl::t_i32 i = 0; i < k_neighbour_pos_offsets.k_len; i++) {
            const auto neighbour_pos = pos + k_neighbour_pos_offsets[i];

            if (neighbour_pos.x < 0 || neighbour_pos.y < 0 || neighbour_pos.x >= lightmap->size.x || neighbour_pos.y >= lightmap->size.y) {
                continue;
            }

            const t_light_level neighbour_light_lvl_new = lightmap->lvls[pos.y][pos.x] - 1;

            // Only brighten a neighbour, don't darken it.
            if (lightmap->lvls[neighbour_pos.y][neighbour_pos.x] < neighbour_light_lvl_new) {
                lightmap->lvls[neighbour_pos.y][neighbour_pos.x] = neighbour_light_lvl_new;

                // No point in adding it to the queue it if it's completely dark, as there'd be no brightness to propagate.
                if (lightmap->lvls[neighbour_pos.y][neighbour_pos.x] > 0) {
                    EnqueueLightPos(&light_pos_queue, neighbour_pos, lightmap->size);
                }
            }
        }
    }

    return true;
}

void LightmapRender(const t_lightmap *const lightmap, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const zcl::t_f32 tile_size) {
    for (zcl::t_i32 ty = 0; ty < lightmap->size.y; ty++) {
        for (zcl::t_i32 tx = 0; tx < lightmap->size.x; tx++) {
            const auto light_lvl = lightmap->lvls[ty][tx];

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
