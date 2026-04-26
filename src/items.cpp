#include "items.h"

#include "tiles.h"
#include "stray.h"
#include "player.h"

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

static void MeleeAttack(const t_item_type_use_func_context &context) {
    auto std_out = zcl::FileStreamCreateStdOut();
    zcl::PrintFormat(zcl::FileStreamGetView(&std_out), ZCL_STR_LITERAL("Attack!"));
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
}}; // @todo: Generally speaking, need some ability static assert on static array length! This is a VERY USEFUL feature!
