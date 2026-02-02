#pragma once

#include "assets.h"

enum t_sprite_id : zcl::t_i32 {
    ek_sprite_id_player,

    ek_sprite_id_npc_slime,

    ek_sprite_id_tile_dirt,
    ek_sprite_id_tile_stone,
    ek_sprite_id_tile_grass,
    ek_sprite_id_tile_hurt_0,
    ek_sprite_id_tile_hurt_1,
    ek_sprite_id_tile_hurt_2,
    ek_sprite_id_tile_hurt_3,

    ek_sprite_id_item_icon_dirt_block,
    ek_sprite_id_item_icon_stone_block,
    ek_sprite_id_item_icon_grass_block,
    ek_sprite_id_item_icon_copper_pickaxe,

    ek_sprite_id_projectile,

    ek_sprite_id_particle_dirt,
    ek_sprite_id_particle_stone,
    ek_sprite_id_particle_grass,
    ek_sprite_id_particle_gel,

    ek_sprite_id_cursor,

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
    {.texture_id = ek_texture_id_tiles, .src_rect = {0, 8, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {8, 8, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {16, 8, 8, 8}},
    {.texture_id = ek_texture_id_tiles, .src_rect = {24, 8, 8, 8}},

    {.texture_id = ek_texture_id_item_icons, .src_rect = {1, 1, 6, 6}},
    {.texture_id = ek_texture_id_item_icons, .src_rect = {9, 1, 6, 6}},
    {.texture_id = ek_texture_id_item_icons, .src_rect = {17, 1, 6, 6}},
    {.texture_id = ek_texture_id_item_icons, .src_rect = {2, 9, 12, 14}},

    {.texture_id = ek_texture_id_projectiles, .src_rect = {0, 2, 16, 4}},

    {.texture_id = ek_texture_id_particles, .src_rect = {2, 2, 4, 4}},
    {.texture_id = ek_texture_id_particles, .src_rect = {10, 2, 4, 4}},
    {.texture_id = ek_texture_id_particles, .src_rect = {18, 2, 4, 4}},
    {.texture_id = ek_texture_id_particles, .src_rect = {26, 2, 4, 4}},

    {.texture_id = ek_texture_id_misc, .src_rect = {2, 2, 4, 4}},
}};

inline void SpriteRender(const t_sprite_id sprite_id, const zgl::t_rendering_context rc, const t_assets *const assets, const zcl::t_v2 pos, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f, const zcl::t_v2 scale = {1.0f, 1.0f}) {
    zgl::RendererSubmitTexture(rc, GetTexture(assets, k_sprites[sprite_id].texture_id), pos, k_sprites[sprite_id].src_rect, origin, rot, scale);
}

inline zcl::t_rect_f ColliderCreate(const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin) {
    ZCL_ASSERT(size.x > 0.0f && size.y > 0.0f);
    ZCL_ASSERT(zcl::OriginCheckValid(origin));

    return zcl::RectCreateF(pos - zcl::CalcCompwiseProd(size, origin), size);
}

inline zcl::t_rect_f ColliderCreateFromSprite(const t_sprite_id sprite_id, const zcl::t_v2 pos, const zcl::t_v2 origin) {
    return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[sprite_id].src_rect)), origin);
}
