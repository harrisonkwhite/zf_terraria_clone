#include "clouds.h"

#include "sprites.h"
#include "camera.h"

struct t_cloud {
    zcl::t_i32 spr_index;
    zcl::t_v2 pos;
    zcl::t_f32 rot_offs;
    zcl::t_f32 scale_offs;
    zcl::t_f32 alpha_offs;
};

struct t_cloud_manager {
    zcl::t_v2 span;

    struct {
        zcl::t_array_mut<zcl::t_array_mut<t_cloud>> clouds;
        zcl::t_array_mut<zcl::t_f32> depths;
    } layers;
};

t_cloud_manager *CloudsCreate(const zcl::t_v2 span, const zcl::t_array_rdonly<zcl::t_f32> layer_depths, zcl::t_rng *const rng, zcl::t_arena *const arena) {
    ZCL_ASSERT(zcl::CheckSorted(layer_depths));

    const auto result = zcl::ArenaPush<t_cloud_manager>(arena);

    result->span = span;

    result->layers.clouds = zcl::ArenaPushArray<zcl::t_array_mut<t_cloud>>(arena, layer_depths.len);

    for (zcl::t_i32 i = 0; i < result->layers.clouds.len; i++) {
        result->layers.clouds[i] = zcl::ArenaPushArray<t_cloud>(arena, 512); // @temp: Hard-coding a count for now.

        for (zcl::t_i32 j = 0; j < result->layers.clouds[i].len; j++) {
            const auto cloud = &result->layers.clouds[i][j];

            cloud->pos = {
                zcl::RandGenPerc(rng) * span.x,
                zcl::RandGenPerc(rng) * span.y,
            };
        }
    }

    result->layers.depths = zcl::ArenaPushArrayClone(arena, layer_depths);

    return result;
}

void CloudsRender(const t_cloud_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera) {
    for (zcl::t_i32 i = 0; i < manager->layers.depths.len; i++) {
        const auto depth = manager->layers.depths[i];
        const auto clouds = manager->layers.clouds[i];

        const auto camera_view_matrix = CameraCalcViewMatrix(camera, rc.screen_size, depth);
        zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix);

        for (zcl::t_i32 j = 0; j < clouds.len; j++) {
            const auto cloud = &clouds[j];
            SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_cloud_0 + cloud->spr_index), rc, assets, cloud->pos, zcl::k_origin_center);
        }

        zgl::RendererPassEnd(rc);
    }
}
