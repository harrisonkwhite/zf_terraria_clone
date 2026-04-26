#pragma once

// @todo: Might want to reconsider this organization if accounting for caching (colliders are probably better grouped together in the same array).
struct t_hitbox {
    zcl::t_rect_f collider; // @todo: Support polygons.
    zcl::t_i32 dmg;
};

struct t_hitbox_manager;

t_hitbox_manager *HitboxManagerCreate(const zcl::t_i32 hitbox_limit, zcl::t_arena *const arena);

void HitboxSubmit(t_hitbox_manager *const manager, const zcl::t_rect_f collider, const zcl::t_i32 dmg);

zcl::t_array_rdonly<t_hitbox> HitboxesLoadAll(const t_hitbox_manager *const manager);

void HitboxesClear(t_hitbox_manager *const manager);
