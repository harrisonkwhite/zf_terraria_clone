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

struct t_cloud_layer {
    zcl::t_v2 span;
    zcl::t_f32 parallax;
    zcl::t_f32 scale;
    zcl::t_f32 alpha;
    zcl::t_array_mut<t_cloud> clouds;
};

t_cloud_layer *CloudLayerCreate(const zcl::t_v2 span, const zcl::t_f32 parallax, const zcl::t_f32 scale, const zcl::t_f32 alpha, zcl::t_rng *const rng, zcl::t_arena *const arena) {
    ZCL_ASSERT(parallax >= 0.0f && parallax <= 1.0f);
    ZCL_ASSERT(scale > 0.0f && scale <= 1.0f);
    ZCL_ASSERT(alpha >= 0.0f && alpha <= 1.0f);

    const auto result = zcl::ArenaPush<t_cloud_layer>(arena);

    result->span = span;
    result->parallax = parallax;
    result->scale = scale;
    result->alpha = alpha;

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

void CloudLayerUpdate(t_cloud_layer *const layer) {
    for (zcl::t_i32 i = 0; i < layer->clouds.len; i++) {
        layer->clouds[i].pos.x += 0.02f;

        const auto spr_id = static_cast<t_sprite_id>(ek_sprite_id_cloud_0 + layer->clouds[i].spr_index);
        const auto spr_width = k_sprites[spr_id].src_rect.width;

        if (layer->clouds[i].pos.x > layer->span.x + spr_width) {
            layer->clouds[i].pos.x = -spr_width;
        }
    }
}

void CloudLayerRender(const t_cloud_layer *const layer, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera) {
    const auto camera_view_matrix = CameraCalcViewMatrix(camera, rc.screen_size, layer->parallax);
    zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix);

    for (zcl::t_i32 i = 0; i < layer->clouds.len; i++) {
        const auto cloud = &layer->clouds[i];
        SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_cloud_0 + cloud->spr_index), rc, assets, cloud->pos, zcl::k_origin_center, cloud->rot, {layer->scale * cloud->scale, layer->scale * cloud->scale}, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, layer->alpha * cloud->alpha));
    }

    zgl::RendererPassEnd(rc);
}
