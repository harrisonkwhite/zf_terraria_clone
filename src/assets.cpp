#include "assets.h"

constexpr zcl::t_u64 k_validation_magic_correct = 0xCAFEBABECAFEBABE;

struct t_assets {
    zcl::t_u64 validation_magic;

    zgl::t_gfx_resource_group *resource_group;

    zcl::t_static_array<zgl::t_gfx_resource *, ekm_texture_id_cnt> textures;
    zcl::t_static_array<zgl::t_font, ekm_font_id_cnt> fonts;
    zcl::t_static_array<zgl::t_gfx_resource *, k_cloud_texture_cnt> cloud_textures;
};

t_assets *AssetsCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
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
        constexpr zcl::t_v2_i k_texture_size = {32, 32};

        const auto px_data = zcl::ArenaPushArray<zcl::t_color_rgba32f>(temp_arena, k_texture_size.x * k_texture_size.y);
        zcl::SetAllTo(px_data, zcl::k_color_white);

        const zcl::t_texture_data_mut texture_data = {
            .dims = k_texture_size,
            .format = zcl::ek_texture_format_rgba32f,
            .pixels = {.rgba32f = px_data},
        };

        for (zcl::t_i32 i = 0; i < k_cloud_texture_cnt; i++) {
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
