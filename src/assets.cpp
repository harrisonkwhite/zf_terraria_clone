#include "assets.h"

constexpr zcl::t_u64 k_validation_magic_correct = 0xCAFEBABECAFEBABE;

struct t_assets {
    zcl::t_u64 validation_magic;

    zgl::t_gfx_resource_group *resource_group;

    zcl::t_static_array<zgl::t_gfx_resource *, ekm_texture_id_cnt> textures;
    zcl::t_static_array<zgl::t_font, ekm_font_id_cnt> fonts;
    zcl::t_static_array<zgl::t_gfx_resource *, k_cloud_texture_cnt> cloud_textures;
};

// @todo: Clouds all need to be in the same texture atlas, the texture swapping is causing way too much slowdown.
static zcl::t_texture_data_mut CloudTextureDataCreate(zcl::t_rng *const rng, zcl::t_arena *const arena) {
    constexpr zcl::t_v2_i k_texture_size = {160, 96};

    const auto px_data = zcl::ArenaPushArray<zcl::t_color_rgba32f>(arena, k_texture_size.x * k_texture_size.y);

    constexpr zcl::t_i32 k_cloud_width_min = 80;
    constexpr zcl::t_i32 k_cloud_width_max = 152;
    static_assert(k_cloud_width_min <= k_cloud_width_max && k_cloud_width_max <= k_texture_size.x);

    const zcl::t_i32 cloud_width = zcl::RandGenI32InRange(rng, k_cloud_width_min / 2, k_cloud_width_max / 2) * 2;

    const auto write_circle = [px_data](const zcl::t_v2_i pos, const zcl::t_i32 radius) {
        ZCL_ASSERT(pos.x >= 0 && pos.y >= 0 && pos.x < k_texture_size.x && pos.y < k_texture_size.y);
        ZCL_ASSERT(radius >= 1);

        const zcl::t_i32 x_min = zcl::CalcMax(pos.x - radius, 0);
        const zcl::t_i32 x_max = zcl::CalcMin(pos.x + radius, k_texture_size.x);

        const zcl::t_i32 y_min = zcl::CalcMax(pos.y - radius, 0);
        const zcl::t_i32 y_max = zcl::CalcMin(pos.y + radius, k_texture_size.y);

        for (zcl::t_i32 y = y_min; y < y_max; y++) {
            for (zcl::t_i32 x = x_min; x < x_max; x++) {
                if (zcl::CalcDist({static_cast<zcl::t_f32>(x) + 0.5f, static_cast<zcl::t_f32>(y) + 0.5f}, zcl::V2IToF(pos)) <= radius) {
                    px_data[(y * k_texture_size.x) + x] = zcl::k_color_white;
                }
            }
        }
    };

    {
        const auto radius_calc = [](const zcl::t_i32 xo) {
            return xo;
        };

        zcl::t_i32 radius = zcl::RandGenI32InRange(rng, 4, 7);
        zcl::t_i32 xo = radius;

        while (xo + radius <= cloud_width / 2) {
            write_circle({xo, (k_texture_size.y / 2) + zcl::RandGenI32InRange(rng, -1, 2)}, radius);
            xo += radius / 2;
            radius += zcl::RandGenI32InRange(rng, 1, 5);
            xo += radius / 2;
        }

        while (xo + radius <= cloud_width && radius >= 1) {
            write_circle({xo, (k_texture_size.y / 2) + zcl::RandGenI32InRange(rng, -1, 2)}, radius);
            xo += radius / 2;
            radius -= zcl::RandGenI32InRange(rng, 1, 5);
            xo += radius / 2;
        }
    }

    return {
        .dims = k_texture_size,
        .format = zcl::ek_texture_format_rgba32f,
        .pixels = {.rgba32f = px_data},
    };
}

t_assets *AssetsCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_rng *const rng, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    const auto result = zcl::ArenaPush<t_assets>(arena);

    result->validation_magic = k_validation_magic_correct;

    result->resource_group = zgl::GFXResourceGroupCreate(gfx_ticket, arena);

    for (zcl::t_i32 i = 0; i < ekm_texture_id_cnt; i++) {
        result->textures[i] = zgl::TextureCreateFromBuilt(gfx_ticket, g_texture_file_paths[i], result->resource_group, temp_arena);
    }

    for (zcl::t_i32 i = 0; i < ekm_font_id_cnt; i++) {
        result->fonts[i] = zgl::FontCreateFromBuilt(gfx_ticket, g_font_file_paths[i], result->resource_group, temp_arena);
    }

    {
        constexpr zcl::t_v2_i k_texture_size = {160, 96};

        const auto px_data = zcl::ArenaPushArray<zcl::t_color_rgba32f>(temp_arena, k_texture_size.x * k_texture_size.y);
        zcl::SetAllTo(px_data, zcl::k_color_white);

        const zcl::t_texture_data_mut texture_data = {
            .dims = k_texture_size,
            .format = zcl::ek_texture_format_rgba32f,
            .pixels = {.rgba32f = px_data},
        };

        for (zcl::t_i32 i = 0; i < k_cloud_texture_cnt; i++) {
            const auto texture_data = CloudTextureDataCreate(rng, temp_arena);
            result->cloud_textures[i] = zgl::TextureCreate(gfx_ticket, texture_data, result->resource_group);
        }
    }

    return result;
}

void AssetsDestroy(t_assets *const assets, const zgl::t_gfx_ticket_mut gfx_ticket) {
    ZCL_ASSERT(assets->validation_magic == k_validation_magic_correct);

    zgl::GFXResourceGroupDestroy(gfx_ticket, assets->resource_group);
    *assets = {};
}

zgl::t_gfx_resource *TextureGet(const t_assets *const assets, const t_texture_id id) {
    ZCL_ASSERT(assets->validation_magic == k_validation_magic_correct);
    return assets->textures[id];
}

const zgl::t_font *FontGet(const t_assets *const assets, const t_font_id id) {
    ZCL_ASSERT(assets->validation_magic == k_validation_magic_correct);
    return &assets->fonts[id];
}

zgl::t_gfx_resource *CloudTextureGet(const t_assets *const assets, const zcl::t_i32 index) {
    ZCL_ASSERT(index >= 0 && index < k_cloud_texture_cnt);
    return assets->cloud_textures[index];
}
