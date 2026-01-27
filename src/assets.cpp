#include "assets.h"

constexpr zcl::t_u64 k_valid_magic_correct = 0xCAFEBABECAFEBABE;

struct t_assets {
    zcl::t_u64 valid_magic;

    zgl::t_gfx_resource_group *resource_group;

    zcl::t_static_array<zgl::t_gfx_resource *, ekm_texture_id_cnt> textures;
    zcl::t_static_array<zgl::t_gfx_resource *, ekm_font_id_cnt> fonts;
};

t_assets *AssetsCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    const auto result = zcl::ArenaPush<t_assets>(arena);

    result->valid_magic = k_valid_magic_correct;

    result->resource_group = zgl::GFXResourceGroupCreate(gfx_ticket, arena);

    for (zcl::t_i32 i = 0; i < ekm_texture_id_cnt; i++) {
        result->textures[i] = zgl::TextureCreateFromBuilt(gfx_ticket, g_texture_file_paths[i], result->resource_group, temp_arena);
    }

    return result;
}

void AssetsDestroy(t_assets *const assets, const zgl::t_gfx_ticket_mut gfx_ticket) {
    ZCL_ASSERT(assets->valid_magic == k_valid_magic_correct);

    zgl::GFXResourceGroupDestroy(gfx_ticket, assets->resource_group);
    *assets = {};
}

zgl::t_gfx_resource *GetTexture(const t_assets *const assets, const t_texture_id id) {
    ZCL_ASSERT(assets->valid_magic == k_valid_magic_correct);
    return assets->textures[id];
}

zgl::t_gfx_resource *GetFont(const t_assets *const assets, const t_font_id id) {
    ZCL_ASSERT(assets->valid_magic == k_valid_magic_correct);
    return assets->fonts[id];
}
