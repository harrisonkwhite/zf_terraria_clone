#include "world_private.h"

#include "camera.h"

namespace world {
    static zcl::t_b8 AddTileAtCursor(const t_item_type_use_func_context &context, const t_tile_type_id tile_type_id) {
        const zcl::t_v2_i tile_hovered_pos = zcl::V2FToI(ScreenToCameraPos(context.cursor_pos, context.screen_size, context.camera) / k_tile_size);

        if (TilemapCheck(context.tilemap, tile_hovered_pos)) {
            return false;
        }

        TilemapAdd(context.tilemap, tile_hovered_pos, tile_type_id);

        return true;
    }

    static zcl::t_b8 HurtTileAtCursor(const t_item_type_use_func_context &context, const zcl::t_i32 damage) {
        const zcl::t_v2_i tile_hovered_pos = zcl::V2FToI(ScreenToCameraPos(context.cursor_pos, context.screen_size, context.camera) / k_tile_size);

        if (!TilemapCheck(context.tilemap, tile_hovered_pos)) {
            return false;
        }

        HurtTile(context.tilemap, tile_hovered_pos, damage);

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
}
