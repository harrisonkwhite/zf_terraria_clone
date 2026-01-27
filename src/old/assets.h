#ifndef ASSETS_H
#define ASSETS_H

#include <stdbool.h>
#include <assert.h>
#include <zfwc.h>

typedef enum {
    ek_texture_player,
    ek_texture_npcs,
    ek_texture_tiles,
    ek_texture_item_icons,
    ek_texture_projectiles,
    ek_texture_particles,
    ek_texture_misc,

    eks_texture_cnt
} e_texture;

static s_rgba_texture GenTextureRGBA(const t_s32 tex_index, s_mem_arena* const mem_arena) {
    switch ((e_texture)tex_index) {
        case ek_texture_player:
            return LoadRGBATextureFromPackedFile((s_char_array_view)ARRAY_FROM_STATIC("assets/textures/player"), mem_arena);

        case ek_texture_npcs:
            return LoadRGBATextureFromPackedFile((s_char_array_view)ARRAY_FROM_STATIC("assets/textures/npcs"), mem_arena);

        case ek_texture_tiles:
            return LoadRGBATextureFromPackedFile((s_char_array_view)ARRAY_FROM_STATIC("assets/textures/tiles"), mem_arena);

        case ek_texture_item_icons:
            return LoadRGBATextureFromPackedFile((s_char_array_view)ARRAY_FROM_STATIC("assets/textures/item_icons"), mem_arena);

        case ek_texture_projectiles:
            return LoadRGBATextureFromPackedFile((s_char_array_view)ARRAY_FROM_STATIC("assets/textures/projectiles"), mem_arena);

        case ek_texture_particles:
            return LoadRGBATextureFromPackedFile((s_char_array_view)ARRAY_FROM_STATIC("assets/textures/particles"), mem_arena);

        case ek_texture_misc:
            return LoadRGBATextureFromPackedFile((s_char_array_view)ARRAY_FROM_STATIC("assets/textures/misc"), mem_arena);

        default:
            assert(false && "Texture case not handled!");
            break;
    }
}

typedef enum {
    ek_font_eb_garamond_20,
    ek_font_eb_garamond_24,
    ek_font_eb_garamond_28,
    ek_font_eb_garamond_32,
    ek_font_eb_garamond_48,
    ek_font_eb_garamond_80,

    eks_font_cnt
} e_font;

const static s_char_array_view g_font_file_paths[] = {
    [ek_font_eb_garamond_20] = ARRAY_FROM_STATIC("assets/fonts/eb_garamond_20"),
    [ek_font_eb_garamond_24] = ARRAY_FROM_STATIC("assets/fonts/eb_garamond_24"),
    [ek_font_eb_garamond_28] = ARRAY_FROM_STATIC("assets/fonts/eb_garamond_28"),
    [ek_font_eb_garamond_32] = ARRAY_FROM_STATIC("assets/fonts/eb_garamond_32"),
    [ek_font_eb_garamond_48] = ARRAY_FROM_STATIC("assets/fonts/eb_garamond_48"),
    [ek_font_eb_garamond_80] = ARRAY_FROM_STATIC("assets/fonts/eb_garamond_80")
};

STATIC_ARRAY_LEN_CHECK(g_font_file_paths, eks_font_cnt);

typedef enum {
    ek_shader_prog_glow,
    eks_shader_prog_cnt
} e_shader_prog;

const static s_shader_prog_gen_info g_shader_prog_gen_infos[] = {
    [ek_shader_prog_glow] = {.file_path = "assets/shader_progs/glow"}
};

STATIC_ARRAY_LEN_CHECK(g_shader_prog_gen_infos, eks_shader_prog_cnt);

#endif
