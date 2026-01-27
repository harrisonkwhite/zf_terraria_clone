#pragma once

#include "assets.h"

enum t_sprite_id : zcl::t_i32 {
    ek_sprite_id_player,
    ek_sprite_id_enemy,
    ek_sprite_id_tile,

    ekm_sprite_id_cnt
};

struct t_sprite {
    t_texture_id texture_id;
    zcl::t_rect_i src_rect;
};

constexpr zcl::t_static_array<t_sprite, ekm_sprite_id_cnt> k_sprites = {{
    {.texture_id = ek_texture_id_all, .src_rect = {0, 0, 16, 24}},
    {.texture_id = ek_texture_id_all, .src_rect = {16, 0, 16, 16}},
    {.texture_id = ek_texture_id_all, .src_rect = {32, 0, 8, 8}},
}};

inline void RendererSubmitSprite(const zgl::t_rendering_context rc, const t_sprite_id sprite_id, const t_assets *const assets, const zcl::t_v2 pos, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f, const zcl::t_v2 scale = {1.0f, 1.0f}) {
    zgl::RendererSubmitTexture(rc, GetTexture(assets, k_sprites[sprite_id].texture_id), pos, k_sprites[sprite_id].src_rect, origin, rot, scale);
}
