#include "sky.h"

#include "assets.h"
#include "camera.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_cloud_spd = 0.5f;
constexpr zcl::t_v2 k_cloud_pos_offs_mult = {0.5f, 0.5f};

struct t_cloud {
    zcl::t_i32 texture_index;
    zcl::t_v2 pos;
    zcl::t_f32 rot_offs;
    zcl::t_f32 scale_offs;
    zcl::t_f32 alpha_offs;
};

struct t_sky_layer {
    zcl::t_f32 parallax;
    zcl::t_list<t_cloud> clouds;
};

struct t_sky {
    zcl::t_array_mut<t_sky_layer> layers;
};

t_sky *SkyCreate(const zcl::t_array_rdonly<t_sky_layer_info> layer_infos, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_rng *const rng, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_sky>(arena);

    result->layers = zcl::ArenaPushArray<t_sky_layer>(arena, layer_infos.len);

    for (zcl::t_i32 i = 0; i < result->layers.len; i++) {
        const auto layer_info = &layer_infos[i];

        ZCL_ASSERT(layer_info->parallax >= 0.0f && layer_info->parallax <= 1.0f);
        ZCL_ASSERT(layer_info->cloud_grid_dims.x > 0 && layer_info->cloud_grid_dims.y > 0);
        ZCL_ASSERT(layer_info->cloud_chance >= 0.0f && layer_info->cloud_chance <= 1.0f);

        const auto layer = &result->layers[i];

        layer->parallax = layer_info->parallax;

        const auto camera_rect = CameraCalcRect(camera, screen_size);

        layer->clouds = zcl::ListCreate<t_cloud>(layer_info->cloud_grid_dims.x * layer_info->cloud_grid_dims.y, arena);

        const auto cloud_gap = zcl::t_v2{
            camera_rect.width / layer_info->cloud_grid_dims.x,
            camera_rect.height / layer_info->cloud_grid_dims.y,
        };

        for (zcl::t_i32 y = 0; y < layer_info->cloud_grid_dims.y; y++) {
            for (zcl::t_i32 x = 0; x < layer_info->cloud_grid_dims.x; x++) {
                if (zcl::RandGenPerc(rng) >= layer_info->cloud_chance) {
                    continue;
                }

                auto cloud = t_cloud{};

                cloud.texture_index = zcl::RandGenI32InRange(rng, 0, k_cloud_texture_cnt);

                cloud.pos = {
                    (zcl::RectGetLeft(camera_rect) * layer_info->parallax) + (((x + 0.5f) / (layer_info->cloud_grid_dims.x - 1)) * camera_rect.width),
                    (zcl::RectGetTop(camera_rect) * layer_info->parallax) + (((y + 0.5f) / (layer_info->cloud_grid_dims.y - 1)) * camera_rect.height),
                };

                cloud.pos += {
                    zcl::RandGenF32InRange(rng, -cloud_gap.x, cloud_gap.x) * k_cloud_pos_offs_mult.x,
                    zcl::RandGenF32InRange(rng, -cloud_gap.y, cloud_gap.y) * k_cloud_pos_offs_mult.y,
                };

                cloud.rot_offs = zcl::k_pi * 0.5f * zcl::RandGenF32InRange(rng, -0.02f, 0.02f);
                cloud.scale_offs = zcl::RandGenF32InRange(rng, -0.025f, 0.025f);
                cloud.alpha_offs = zcl::RandGenF32InRange(rng, -0.025f, 0.025f);

                zcl::ListAppend(&layer->clouds, cloud);
            }
        }
    }

    return result;
}

void SkyUpdate(t_sky *const sky, const zgl::t_gfx_ticket_rdonly gfx_ticket, const t_camera *const camera, const zcl::t_v2_i screen_size, const t_assets *const assets) {
    for (zcl::t_i32 i = 0; i < sky->layers.len; i++) {
        const auto layer = sky->layers[i];

        const auto camera_rect = CameraCalcRect(camera, screen_size);

        for (zcl::t_i32 i = 0; i < layer.clouds.len; i++) {
            const auto cloud = &layer.clouds[i];

            cloud->pos.x += k_cloud_spd * layer.parallax;

            // Handle wrapping.
            const auto cloud_camera_pos_calc = [cloud, camera_rect, layer]() {
                return cloud->pos - (zcl::RectGetTopLeft(camera_rect) * layer.parallax) + zcl::RectGetTopLeft(camera_rect);
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
    }
}

void SkyRender(const t_sky *const sky, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera) {
    zgl::RendererPassBegin(rc, rc.screen_size, zcl::MatrixCreateIdentity(), true, k_bg_color);
    zgl::RendererPassEnd(rc);

    for (zcl::t_i32 i = 0; i < sky->layers.len; i++) {
        const auto layer = sky->layers[i];

        const auto camera_rect = CameraCalcRect(camera, rc.screen_size);

        const auto camera_view_matrix = CameraCalcViewMatrix(camera, rc.screen_size, layer.parallax);
        zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix);

        for (zcl::t_i32 i = 0; i < layer.clouds.len; i++) {
            const auto cloud = &layer.clouds[i];

            const zcl::t_f32 scale = zcl::CalcMin(layer.parallax * 10.0f, 1.0f) + cloud->scale_offs;
            const zcl::t_f32 alpha = zcl::Clamp((layer.parallax * 5.0f) + cloud->alpha_offs, 0.0f, 1.0f);

            zgl::RendererSubmitTexture(rc, CloudTextureGet(assets, cloud->texture_index), cloud->pos, {}, zcl::k_origin_center, cloud->rot_offs, {scale, scale}, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, alpha));
        }

        zgl::RendererPassEnd(rc);
    }
}
