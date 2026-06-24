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
#ifdef ZCL_DEBUG
    for (zcl::t_i32 i = 0; i < layer_depths.len; i++) {
        ZCL_ASSERT(layer_depths[i] > 0.0f && layer_depths[i] <= 1.0f);

        if (i > 0) {
            ZCL_ASSERT(layer_depths[i] > layer_depths[i - 1]);
        }
    }
#endif

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

void CloudsUpdate(t_cloud_manager *const manager) {
    for (zcl::t_i32 i = 0; i < manager->layers.clouds.len; i++) {
        const auto layer_clouds = manager->layers.clouds[i];
        const auto layer_depth = manager->layers.depths[i];

        for (zcl::t_i32 j = 0; j < layer_clouds.len; j++) {
            const auto cloud = &layer_clouds[j];

            cloud->pos.x += layer_depth * 0.2f;

            const auto spr_id = static_cast<t_sprite_id>(ek_sprite_id_cloud_0 + cloud->spr_index);
            const auto spr_width = k_sprites[spr_id].src_rect.width;

            if (cloud->pos.x > manager->span.x + spr_width) {
                cloud->pos.x = -spr_width;
            }
        }
    }
}

void CloudsRender(const t_cloud_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera) {
    for (zcl::t_i32 i = 0; i < manager->layers.clouds.len; i++) {
        const auto layer_clouds = manager->layers.clouds[i];
        const auto layer_depth = manager->layers.depths[i];

        const auto camera_view_matrix = CameraCalcViewMatrix(camera, rc.screen_size, layer_depth);
        zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix);

        for (zcl::t_i32 j = 0; j < layer_clouds.len; j++) {
            const auto cloud = &layer_clouds[j];

            const zcl::t_f32 scale = zcl::CalcMin(layer_depth * 8.0f, 1.0f);
            const zcl::t_f32 alpha = zcl::CalcMin(layer_depth * 8.0f, 1.0f);

            SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_cloud_0 + cloud->spr_index), rc, assets, cloud->pos, zcl::k_origin_center, cloud->rot_offs, {scale + cloud->scale_offs, scale + cloud->scale_offs}, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, alpha));
        }

        zgl::RendererPassEnd(rc);
    }
}
