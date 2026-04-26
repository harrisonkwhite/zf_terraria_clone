#pragma once

enum t_hitbox_flags : zcl::t_u8 {
    ek_hitbox_flags_none = 0,

    ek_hitbox_flag_hurt_player = 1 << 0,
    ek_hitbox_flag_hurt_npcs = 1 << 1,
};

struct t_hitbox {
    zcl::t_rect_f collider; // @todo: Support polygons.
    zcl::t_i32 dmg;
    t_hitbox_flags flags;
};

struct t_hitbox_manager;

t_hitbox_manager *HitboxManagerCreate(const zcl::t_i32 hitbox_limit, zcl::t_arena *const arena);

void HitboxSubmit(t_hitbox_manager *const manager, const t_hitbox hitbox);

zcl::t_array_rdonly<t_hitbox> HitboxesLoadAll(const t_hitbox_manager *const manager);

void HitboxesClear(t_hitbox_manager *const manager);
