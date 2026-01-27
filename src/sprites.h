#pragma once

#include "assets.h"

enum t_sprite_id : zcl::t_i32 {
    ek_sprite_id_player,

    ek_sprite_id_slime,

    ek_sprite_id_dirt_tile,
    ek_sprite_id_stone_tile,
    ek_sprite_id_grass_tile,
    ek_sprite_id_sand_tile,
    ek_sprite_id_tile_break_0,
    ek_sprite_id_tile_break_1,
    ek_sprite_id_tile_break_2,
    ek_sprite_id_tile_break_3,

    ek_sprite_id_dirt_block_item_icon,
    ek_sprite_id_stone_block_item_icon,
    ek_sprite_id_grass_block_item_icon,
    ek_sprite_id_sand_block_item_icon,
    ek_sprite_id_copper_pickaxe_item_icon,
    ek_sprite_id_item_icon_template,

    ek_sprite_id_projectile,

    ek_sprite_id_dirt_particle,
    ek_sprite_id_stone_particle,
    ek_sprite_id_grass_particle,
    ek_sprite_id_sand_particle,
    ek_sprite_id_gel_particle,

    ek_sprite_id_mouse,

    ekm_sprite_id_cnt
};

struct t_sprite {
    t_texture_id texture_id;
    zcl::t_rect_i src_rect;
};

constexpr zcl::t_static_array<t_sprite, ekm_sprite_id_cnt> k_sprites = {{
    {.texture_id = ek_texture_id_player, .src_rect = {1, 1, 14, 22}},
    {.texture_id = ek_texture_id_npcs, .src_rect = {1, 1, 14, 14}},

    {.texture_id = ek_texture_id_tiles, .src_rect = {0, 0, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {8, 0, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {16, 0, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {16, 0, 8, 8}}, // @temp
    {.texture_id = ek_texture_id_tiles, .src_rect = {0, 8, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {8, 8, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {16, 8, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {24, 8, 8, 8}},

    {.texture_id = ek_texture_id_item_icons, .src_rect = {1, 1, 6, 6}},
    {.texture_id = ek_texture_id_item_icons, .src_rect = {9, 1, 6, 6}},
    {.texture_id = ek_texture_id_item_icons, .src_rect = {17, 1, 6, 6}},
    {.texture_id = ek_texture_id_item_icons, .src_rect = {2, 9, 12, 14}},
    {.texture_id = ek_texture_id_item_icons, .src_rect = {0, 24, 16, 16}},

    {.texture_id = ek_texture_id_projectiles, .src_rect = {0, 2, 16, 4}},

    {.texture_id = ek_texture_id_particles, .src_rect = {2, 2, 4, 4}},
    {.texture_id = ek_texture_id_particles, .src_rect = {10, 2, 4, 4}},
    {.texture_id = ek_texture_id_particles, .src_rect = {18, 2, 4, 4}},
    {.texture_id = ek_texture_id_particles, .src_rect = {18, 2, 4, 4}},
    {.texture_id = ek_texture_id_particles, .src_rect = {26, 2, 4, 4}},

    {.texture_id = ek_texture_id_misc, .src_rect = {2, 2, 4, 4}},
}};

inline void RendererSubmitSprite(const zgl::t_rendering_context rc, const t_sprite_id sprite_id, const t_assets *const assets, const zcl::t_v2 pos, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f, const zcl::t_v2 scale = {1.0f, 1.0f}) {
    zgl::RendererSubmitTexture(rc, GetTexture(assets, k_sprites[sprite_id].texture_id), pos, k_sprites[sprite_id].src_rect, origin, rot, scale);
}
