#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_assets;

struct t_tilemap;

// ==================================================

constexpr zcl::t_i32 k_npc_limit = 1024;
constexpr zcl::t_v2 k_npc_origin = zcl::k_origin_center;
constexpr zcl::t_i32 k_npc_flash_duration = 10;

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

struct t_npc {
    zcl::t_i32 health;

    zcl::t_v2 pos;

    t_npc_type_id type_id;

    union {
        struct {
            zcl::t_v2 vel;
        } slime;
    } type_data;

    zcl::t_i32 flash_time;
};

// @todo: This should be decoupled. It's important to make sure you can't deactivate NPCs during tick for example.
struct t_npc_manager {
    zcl::t_static_array<t_npc, k_npc_limit> buf;
    zcl::t_static_bitset<k_npc_limit> activity;
    zcl::t_static_array<zcl::t_i32, k_npc_limit> versions;
};

struct t_npc_id {
    zcl::t_i32 index;
    zcl::t_i32 version;
};

t_npc_id SpawnNPC(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id);

void HurtNPC(t_npc_manager *const manager, const t_npc_id id, const zcl::t_i32 damage);

zcl::t_b8 CheckNPCExists(const t_npc_manager *const manager, const t_npc_id id);

zcl::t_rect_f GetNPCCollider(const zcl::t_v2 pos, const t_npc_type_id type_id);

void ProcessNPCAIs(t_npc_manager *const npcs, const t_tilemap *const tilemap);

void ProcessNPCDeaths(t_npc_manager *const npcs);

void RenderNPCs(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets);
