#pragma once

#include "assets.h"

// ============================================================
// @section: External Forward Declarations

struct t_camera;

// ==================================================

constexpr zcl::t_i32 k_pop_up_limit = 1024;
constexpr zcl::t_i32 k_pop_up_life_fade_thresh = 10;
constexpr zcl::t_f32 k_pop_up_lerp_factor = 0.15f;

struct t_pop_up {
    zcl::t_i32 life;

    zcl::t_v2 pos;
    zcl::t_v2 vel;

    zcl::t_static_array<zcl::t_u8, 32> str_bytes;
    zcl::t_i32 str_byte_cnt;

    t_font_id font_id;
};

struct t_pop_up_manager {
    zcl::t_static_array<t_pop_up, k_pop_up_limit> buf;
    zcl::t_static_bitset<k_pop_up_limit> activity; // So this is kind of a second source of truth since you can determine whether a pop-up is active from its life value.
                                                   // But I think this is the ideal approach since it makes lookups for when you want to get the first free index to take faster,
                                                   // and pop-up spawning is something that's going to be happening CONSTANTLY in a game like this.
};

t_pop_up *PopUpSpawn(t_pop_up_manager *const manager, const zcl::t_i32 life, const zcl::t_v2 pos, const zcl::t_v2 vel, const t_font_id font_id = ek_font_id_eb_garamond_32);

t_pop_up *PopUpSpawnDamage(t_pop_up_manager *const manager, const zcl::t_v2 pos, const zcl::t_i32 damage, zcl::t_rng *const rng);

void PopUpsUpdate(t_pop_up_manager *const manager);

void PopUpsRender(const t_pop_up_manager *const pop_ups, const zgl::t_rendering_context rc, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena);
