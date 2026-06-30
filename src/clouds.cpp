#include "clouds.h"

#include "assets.h"
#include "camera.h"

struct t_cloud {
    zcl::t_i32 texture_index;
    zcl::t_v2 pos;
    zcl::t_f32 rot_offs;
    zcl::t_f32 scale_offs;
    zcl::t_f32 alpha_offs;
};

struct t_cloud_layer {
    zcl::t_f32 parallax;
    zcl::t_f32 alpha;
    zcl::t_array_mut<t_cloud> clouds;
};

t_cloud_layer *CloudLayerCreate(const zcl::t_v2_i cloud_cnt, const zcl::t_v2 cloud_pos_offs_mult, const zcl::t_f32 parallax, const zcl::t_f32 alpha, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_rng *const rng, zcl::t_arena *const arena) {
    ZCL_ASSERT(cloud_cnt.x > 0 && cloud_cnt.y > 0);
    ZCL_ASSERT(cloud_pos_offs_mult.x >= 0.0f && cloud_pos_offs_mult.x <= 1.0f && cloud_pos_offs_mult.y >= 0.0f && cloud_pos_offs_mult.y <= 1.0f);
    ZCL_ASSERT(parallax >= 0.0f && parallax <= 1.0f);
    ZCL_ASSERT(alpha >= 0.0f && alpha <= 1.0f);

    const auto result = zcl::ArenaPush<t_cloud_layer>(arena);

    result->parallax = parallax;
    result->alpha = alpha;

    const auto camera_rect = CameraCalcRect(camera, screen_size);

    result->clouds = zcl::ArenaPushArray<t_cloud>(arena, cloud_cnt.x * cloud_cnt.y);

    const auto cloud_gap = zcl::t_v2{
        camera_rect.width / cloud_cnt.x,
        camera_rect.height / cloud_cnt.y,
    };

    for (zcl::t_i32 y = 0; y < cloud_cnt.y; y++) {
        for (zcl::t_i32 x = 0; x < cloud_cnt.x; x++) {
            const auto cloud = &result->clouds[(y * cloud_cnt.x) + x];

            cloud->texture_index = zcl::RandGenI32InRange(rng, 0, k_cloud_texture_cnt);

            cloud->pos = {
                (zcl::RectGetLeft(camera_rect) * parallax) + (((x + 0.5f) / (cloud_cnt.x - 1)) * camera_rect.width),
                (zcl::RectGetTop(camera_rect) * parallax) + (((y + 0.5f) / (cloud_cnt.y - 1)) * camera_rect.height),
            };

            cloud->pos += {
                zcl::RandGenF32InRange(rng, -cloud_gap.x, cloud_gap.x) * cloud_pos_offs_mult.x,
                zcl::RandGenF32InRange(rng, -cloud_gap.y, cloud_gap.y) * cloud_pos_offs_mult.y,
            };

            cloud->rot_offs = zcl::k_pi * 0.5f * zcl::RandGenF32InRange(rng, -0.02f, 0.02f);
            cloud->scale_offs = zcl::RandGenF32InRange(rng, -0.05f, 0.05f);
            cloud->alpha_offs = zcl::RandGenF32InRange(rng, -0.05f, 0.05f);
        }
    }

    return result;
}

void CloudLayerUpdate(t_cloud_layer *const layer, const zgl::t_gfx_ticket_rdonly gfx_ticket, const t_camera *const camera, const zcl::t_v2_i screen_size, const t_assets *const assets) {
#if 0
    // Handle wrapping.
    const auto camera_rect = CameraCalcRect(camera, screen_size);

    for (zcl::t_i32 i = 0; i < layer->clouds.len; i++) {
        const auto cloud = &layer->clouds[i];

        const auto cloud_camera_pos_calc = [cloud, camera_rect, layer]() {
            return cloud->pos - (zcl::RectGetTopLeft(camera_rect) * layer->parallax) + zcl::RectGetTopLeft(camera_rect);
        };

        const auto cloud_texture_size = zgl::TextureGetSize(gfx_ticket, CloudTextureGet(assets, cloud->texture_index));

        while (cloud_camera_pos_calc().x + (cloud_texture_size.x / 2.0f) < zcl::RectGetLeft(camera_rect)) {
            cloud->pos.x += camera_rect.width + cloud_texture_size.x;
        }

        while (cloud_camera_pos_calc().y + (cloud_texture_size.y / 2.0f) < zcl::RectGetTop(camera_rect)) {
            cloud->pos.y += camera_rect.height + cloud_texture_size.y;
        }

        while (cloud_camera_pos_calc().x - (cloud_texture_size.x / 2.0f) >= zcl::RectGetRight(camera_rect)) {
            cloud->pos.x -= camera_rect.width + cloud_texture_size.x;
        }

        while (cloud_camera_pos_calc().y - (cloud_texture_size.y / 2.0f) >= zcl::RectGetBottom(camera_rect)) {
            cloud->pos.y -= camera_rect.height + cloud_texture_size.y;
        }
    }
#endif
}

void CloudLayerRender(const t_cloud_layer *const layer, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera) {
#if 0
    const auto camera_rect = CameraCalcRect(camera, rc.screen_size);

    const auto camera_view_matrix = CameraCalcViewMatrix(camera, rc.screen_size, layer->parallax);
    zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix);

    for (zcl::t_i32 i = 0; i < layer->clouds.len; i++) {
        const auto cloud = &layer->clouds[i];

        const zcl::t_f32 scale = 1.0f + cloud->scale_offs;
        const zcl::t_f32 alpha = zcl::Clamp(layer->alpha + cloud->alpha_offs, 0.0f, 1.0f);

        zgl::RendererSubmitTexture(rc, CloudTextureGet(assets, cloud->texture_index), cloud->pos, {}, zcl::k_origin_center, cloud->rot_offs, {scale, scale}, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, alpha));
    }

    zgl::RendererPassEnd(rc);
#endif
}
