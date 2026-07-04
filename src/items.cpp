#include "items.h"

#include "tiles.h"
#include "stray.h"
#include "player.h"
#include "npcs.h"
#include "hitboxes.h"
#include "camera.h"

static zcl::t_b8 AddTileAtCursor(const t_item_type_use_func_context &context, const t_tile_type_id tile_type_id) {
    zcl::t_v2_i tile_hovered_pos;

    if (!LoadHoveredTilePositionIfInReach(context.cursor_pos, context.screen_size, context.camera, PlayerGetPosition(context.player_entity), &tile_hovered_pos)) {
        return false;
    }

    if (TilemapCheck(context.tilemap, tile_hovered_pos)) {
        return false;
    }

    const auto tile_collider = TilemapGetColliderAt(tile_hovered_pos);

    const auto player_collider = PlayerGetCollider(PlayerGetPosition(context.player_entity));

    if (zcl::CheckInters(tile_collider, player_collider)) {
        return false;
    }

    if (NPCsCheckCollision(context.npc_manager, tile_collider)) {
        return false;
    }

    TilemapPlace(context.tilemap, tile_hovered_pos, tile_type_id);

    return true;
}

static zcl::t_b8 HurtTileAtCursor(const t_item_type_use_func_context &context, const zcl::t_i32 damage) {
    zcl::t_v2_i tile_hovered_pos;

    if (!LoadHoveredTilePositionIfInReach(context.cursor_pos, context.screen_size, context.camera, PlayerGetPosition(context.player_entity), &tile_hovered_pos)) {
        return false;
    }

    if (!TilemapCheck(context.tilemap, tile_hovered_pos)) {
        return false;
    }

    TilemapHurt(context.tilemap, tile_hovered_pos, damage, context.item_drop_manager);

    return true;
}

static void MeleeAttack(const t_item_type_use_func_context &context) {
    const auto player_pos = PlayerGetPosition(context.player_entity);

    const auto cursor_camera_pos = ScreenToCameraPos(context.cursor_pos, context.screen_size, context.camera);

    const auto hitbox_pos = player_pos + zcl::t_v2{16.0f * (cursor_camera_pos.x >= player_pos.x ? 1 : -1), 0.0f};
    const auto hitbox_size = zcl::t_v2{24.0f, 20.0f};

    const auto hitbox = t_hitbox{
        .collider = zcl::RectCreateF(hitbox_pos - (hitbox_size / 2.0f), hitbox_size),
        .dmg = 5,
        .flags = ek_hitbox_flag_hurt_npcs,
    };

    HitboxSubmit(context.hitbox_manager, hitbox);
}

const zcl::t_static_array<t_item_type_use_func, ekm_item_type_id_cnt> g_item_type_use_funcs = {{
    [](const t_item_type_use_func_context &context) {
        return AddTileAtCursor(context, ek_tile_type_id_dirt);
    },
    [](const t_item_type_use_func_context &context) {
        return AddTileAtCursor(context, ek_tile_type_id_stone);
    },
    [](const t_item_type_use_func_context &context) {
        return AddTileAtCursor(context, ek_tile_type_id_grass);
    },
    [](const t_item_type_use_func_context &context) {
        return HurtTileAtCursor(context, 10);
    },
    [](const t_item_type_use_func_context &context) {
        MeleeAttack(context);
        return true;
    },
    [](const t_item_type_use_func_context &context) {
        return false;
    },
}}; // @todo: Generally speaking, need some ability static assert on static array length! This is a VERY USEFUL feature!
