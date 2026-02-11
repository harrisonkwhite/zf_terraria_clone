#include "stray.h"

#include "tiles.h"
#include "camera.h"
#include "player.h"
#include "npcs.h"

constexpr zcl::t_f32 k_make_contact_with_tilemap_jump_size_precise = 0.5f;

static zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap_core *const tilemap) {
    ZCL_ASSERT(jump_size > 0.0f);

    zcl::t_v2 pos_next = pos_current;

    const zcl::t_v2 jump_dir = zcl::k_cardinal_direction_normals[cardinal_dir_id];
    const zcl::t_v2 jump = jump_dir * jump_size;

    while (!TilemapCheckCollision(tilemap, ColliderCreate(pos_next + jump, collider_size, collider_origin))) {
        pos_next += jump;
    }

    return pos_next;
}

zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap_core *const tilemap) {
    zcl::t_v2 pos_next = pos_current;

    // Jump by tile intervals first, then make more precise contact.
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_tile_size, cardinal_dir_id, collider_size, collider_origin, tilemap);
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_make_contact_with_tilemap_jump_size_precise, cardinal_dir_id, collider_size, collider_origin, tilemap);

    return pos_next;
}

static void ProcessTilemapCollisionsVertical(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap_core *const tilemap) {
    const zcl::t_rect_f collider_vertical = ColliderCreate({pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_vertical)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_make_contact_with_tilemap_jump_size_precise, *vel_y >= 0.0f ? zcl::ek_cardinal_direction_down : zcl::ek_cardinal_direction_up, collider_size, collider_origin, tilemap);
        *vel_y = 0.0f;
    }
}

void ProcessTilemapCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap_core *const tilemap) {
    const zcl::t_rect_f collider_hor = ColliderCreate({pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_hor)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_make_contact_with_tilemap_jump_size_precise, vel->x >= 0.0f ? zcl::ek_cardinal_direction_right : zcl::ek_cardinal_direction_left, collider_size, collider_origin, tilemap);
        vel->x = 0.0f;
    }

    ProcessTilemapCollisionsVertical(pos, &vel->y, collider_size, collider_origin, tilemap);

    const zcl::t_rect_f collider_diagonal = ColliderCreate(*pos + *vel, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_diagonal)) {
        vel->x = 0.0f;
    }
}

zcl::t_rect_i CalcCameraTilemapRect(const t_camera *const camera, const t_tilemap_core *const tilemap, const zcl::t_v2_i screen_size) {
    ZCL_ASSERT(screen_size.x > 0 && screen_size.y > 0);

    const zcl::t_f32 camera_scale = CameraGetScale(camera);

    const zcl::t_rect_f camera_rect = CameraCalcRect(camera, screen_size);

    const zcl::t_i32 camera_tilemap_left = static_cast<zcl::t_i32>(zcl::Floor(zcl::RectGetLeft(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_top = static_cast<zcl::t_i32>(zcl::Floor(zcl::RectGetTop(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_right = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetRight(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_bottom = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetBottom(camera_rect) / k_tile_size));

    return zcl::ClampWithinContainer(zcl::RectCreateI(camera_tilemap_left, camera_tilemap_top, camera_tilemap_right - camera_tilemap_left, camera_tilemap_bottom - camera_tilemap_top), zcl::RectCreateI({}, TilemapGetSize(tilemap)));
}

void ProcessPlayerAndNPCCollisions(t_player_entity *const player_entity, const t_npc_manager *const npc_manager, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(PlayerCheckAlive(player_entity));

    const auto player_collider = PlayerGetCollider(PlayerGetPosition(player_entity));

    const auto npcs = NPCsLoad(npc_manager, temp_arena);

    for (zcl::t_i32 i = 0; i < npcs.len; i++) {
        const auto npc = npcs[i];
        const auto npc_type = &g_npc_types[npc->type_id];

        if (!npc_type->touch_hurt) {
            continue;
        }

        const auto npc_collider = NPCGetCollider(npc->pos, npc->type_id);

        if (zcl::CheckInters(player_collider, npc_collider)) {
            PlayerHurt(player_entity, npc_type->touch_hurt_damage, pop_up_manager, rng);
        }
    }
}
