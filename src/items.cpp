#include "items.h"

#include "tiles.h"
#include "camera.h"
#include "player.h"

[[nodiscard]] static zcl::t_b8 LoadHoveredTilePositionIfInReach(const zcl::t_v2 cursor_pos, const zcl::t_v2_i screen_size, const t_camera *const camera, const zcl::t_v2 player_pos, zcl::t_v2_i *const o_pos) {
    const zcl::t_v2_i player_tile_pos = {
        static_cast<zcl::t_i32>(zcl::Floor(player_pos.x / k_tile_size)),
        static_cast<zcl::t_i32>(zcl::Floor(player_pos.y / k_tile_size)),
    };

    *o_pos = zcl::V2FToI(ScreenToCameraPos(cursor_pos, screen_size, camera) / k_tile_size);

    if (zcl::CalcDist(zcl::V2IToF(player_tile_pos), zcl::V2IToF(*o_pos)) > k_item_tile_reach_dist) {
        return false;
    }

    return true;
}

static zcl::t_b8 AddTileAtCursor(const t_item_type_use_func_context &context, const t_tile_type_id tile_type_id) {
    zcl::t_v2_i tile_hovered_pos;

    if (!LoadHoveredTilePositionIfInReach(context.cursor_pos, context.screen_size, context.camera, PlayerGetPosition(context.player_entity), &tile_hovered_pos)) {
        return false;
    }

    if (TilemapCheck(context.tilemap, tile_hovered_pos)) {
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
}}; // @todo: Generally speaking, need some ability static assert on static array length! This is a VERY USEFUL feature!
