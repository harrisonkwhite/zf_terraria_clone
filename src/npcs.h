#pragma once

#include "hitboxes.h"

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_player_entity;

struct t_tilemap;

struct t_hitbox;

struct t_pop_up_manager;

// ==================================================

enum t_npc_type_id : zcl::t_i32 {
    ek_npc_type_id_slime,

    ekm_npc_type_id_cnt
};

struct t_npc_type {
    zcl::t_str_rdonly name;

    zcl::t_i32 health_limit;

    zcl::t_b8 touch_hurt;
    zcl::t_i32 touch_hurt_damage;
    zcl::t_f32 touch_hurt_force_mag;
};

inline const zcl::t_static_array<t_npc_type, ekm_npc_type_id_cnt> g_npc_types = {{
    {
        .name = ZCL_STR_LITERAL("Slime"),
        .health_limit = 30,
        .touch_hurt = true,
        .touch_hurt_damage = 10,
    },
}};

struct t_npc_manager;

struct t_npc_id {
    zcl::t_i32 index;
    zcl::t_i32 version;
};

t_npc_manager *NPCManagerCreate(zcl::t_arena *const arena);

t_npc_id NPCSpawn(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id, zcl::t_rng *const rng);

zcl::t_b8 NPCCheckExists(const t_npc_manager *const manager, const t_npc_id id);

zcl::t_v2 NPCGetPosition(const t_npc_manager *const manager, const t_npc_id id);

t_npc_type_id NPCGetTypeID(const t_npc_manager *const manager, const t_npc_id id);

zcl::t_rect_f NPCGetCollider(const zcl::t_v2 pos, const t_npc_type_id type_id);

void NPCsProcessAIs(t_npc_manager *const manager, const zcl::t_f32 gravity, const t_player_entity *const player_entity, const t_tilemap *const tilemap, zcl::t_rng *const rng);

void NPCsSubmitHitboxes(const t_npc_manager *const npc_manager, t_hitbox_manager *const hitbox_manager);

void NPCsProcessHitboxCollisions(t_npc_manager *const npc_manager, const zcl::t_array_rdonly<t_hitbox> hitboxes, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng);

void NPCsProcessDeaths(t_npc_manager *const manager);

void NPCsRender(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets);

zcl::t_array_mut<t_npc_id> NPCsLoad(const t_npc_manager *const manager, zcl::t_arena *const arena);
