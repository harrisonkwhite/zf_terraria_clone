#pragma once

enum t_texture_id : zcl::t_i32 {
    ek_texture_id_player,
    ek_texture_id_npcs,
    ek_texture_id_tiles,
    ek_texture_id_items,
    ek_texture_id_projectiles,
    ek_texture_id_particles,
    ek_texture_id_misc,

    ekm_texture_id_cnt
};

const zcl::t_static_array<zcl::t_str_rdonly, ekm_texture_id_cnt> g_texture_file_paths = {{
    ZCL_STR_LITERAL("assets/textures/player.bin"),
    ZCL_STR_LITERAL("assets/textures/npcs.bin"),
    ZCL_STR_LITERAL("assets/textures/tiles.bin"),
    ZCL_STR_LITERAL("assets/textures/items.bin"),
    ZCL_STR_LITERAL("assets/textures/projectiles.bin"),
    ZCL_STR_LITERAL("assets/textures/particles.bin"),
    ZCL_STR_LITERAL("assets/textures/misc.bin"),
}};

enum t_font_id : zcl::t_i32 {
    ek_font_id_roboto_20,
    ek_font_id_roboto_24,
    ek_font_id_roboto_28,
    ek_font_id_roboto_32,
    ek_font_id_roboto_40,
    ek_font_id_roboto_64,
    ek_font_id_roboto_184,

    ekm_font_id_cnt
};

const zcl::t_static_array<zcl::t_str_rdonly, ekm_font_id_cnt> g_font_file_paths = {{
    ZCL_STR_LITERAL("assets/fonts/roboto_20.bin"),
    ZCL_STR_LITERAL("assets/fonts/roboto_24.bin"),
    ZCL_STR_LITERAL("assets/fonts/roboto_28.bin"),
    ZCL_STR_LITERAL("assets/fonts/roboto_32.bin"),
    ZCL_STR_LITERAL("assets/fonts/roboto_40.bin"),
    ZCL_STR_LITERAL("assets/fonts/roboto_64.bin"),
    ZCL_STR_LITERAL("assets/fonts/roboto_184.bin"),
}};

constexpr zcl::t_i32 k_cloud_texture_cnt = 32;

struct t_assets;

t_assets *AssetsCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);

void AssetsDestroy(t_assets *const assets, const zgl::t_gfx_ticket_mut gfx_ticket);

zgl::t_gfx_resource *TextureGet(const t_assets *const assets, const t_texture_id id);

const zgl::t_font *FontGet(const t_assets *const assets, const t_font_id id);

zgl::t_gfx_resource *CloudTextureGet(const t_assets *const assets, const zcl::t_i32 index);
