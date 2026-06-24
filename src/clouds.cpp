#include "clouds.h"

#include "sprites.h"
#include "camera.h"

struct t_cloud {
    zcl::t_i32 spr_index;
    zcl::t_v2 pos;
    zcl::t_f32 rot;
    zcl::t_f32 scale;
    zcl::t_f32 alpha;
};

struct t_cloud_manager {
    zcl::t_v2 span;
    zcl::t_array_mut<t_cloud> clouds;
};

t_cloud_manager *CloudManagerCreate(const zcl::t_v2 span, zcl::t_rng *const rng, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_cloud_manager>(arena);
    result->clouds = zcl::ArenaPushArray<t_cloud>(arena, 512);

    for (zcl::t_i32 i = 0; i < result->clouds.len; i++) {
        const auto cloud = &result->clouds[i];

        cloud->pos = {
            zcl::RandGenPerc(rng) * span.x,
            zcl::RandGenPerc(rng) * span.y,
        };

        cloud->rot = zcl::k_pi * zcl::RandGenF32InRange(rng, -0.01f, 0.01f);
        cloud->scale = zcl::RandGenF32InRange(rng, 0.8f, 1.0f);
        cloud->alpha = 1.0f;
    }

    return result;
}

void CloudManagerUpdateAll(t_cloud_manager *const manager) {
    for (zcl::t_i32 i = 0; i < manager->clouds.len; i++) {
        manager->clouds[i].pos.x += 0.02f;

        const auto spr_id = static_cast<t_sprite_id>(ek_sprite_id_cloud_0 + manager->clouds[i].spr_index);
        const auto spr_width = k_sprites[spr_id].src_rect.width;

        if (manager->clouds[i].pos.x > manager->span.x + spr_width) {
            manager->clouds[i].pos.x = -spr_width;
        }
    }
}

void CloudManagerRenderAll(const t_cloud_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera) {
    const auto camera_view_matrix = CameraCalcViewMatrix(camera, rc.screen_size, 0.05f);
    zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix);

    for (zcl::t_i32 i = 0; i < manager->clouds.len; i++) {
        const auto cloud = &manager->clouds[i];
        SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_cloud_0 + cloud->spr_index), rc, assets, cloud->pos, zcl::k_origin_center, cloud->rot, {cloud->scale, cloud->scale}, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, cloud->alpha));
    }

    zgl::RendererPassEnd(rc);
}
