#include "world_private.h"

#include "tiles.h"
#include "camera.h"

namespace world {
    struct t_item_type_use_func_context {
        t_world *world;
        zcl::t_v2 cursor_pos;
        zcl::t_v2_i screen_size;
        zcl::t_arena *temp_arena;
    };

    using t_item_type_use_func = zcl::t_b8 (*)(const t_item_type_use_func_context &context);

    static zcl::t_b8 AddTileAtCursor(const t_item_type_use_func_context &context, const t_tile_type_id tile_type_id) {
#if 0
        const zcl::t_v2_i tile_hovered_pos = zcl::V2FToI(ScreenToCameraPos(context.cursor_pos, context.screen_size, context.world->camera) / k_tile_size);

        if (CheckTile(context.world->tilemap, tile_hovered_pos)) {
            return false;
        }

        AddTile(context.world->tilemap, tile_hovered_pos, tile_type_id);
#endif

        return true;
    }

    static zcl::t_b8 HurtTileAtCursor(const t_item_type_use_func_context &context, const zcl::t_i32 damage) {
#if 0
        const zcl::t_v2_i tile_hovered_pos = zcl::V2FToI(ScreenToCameraPos(context.cursor_pos, context.screen_size, context.world->camera) / k_tile_size);

        if (!CheckTile(context.world->tilemap, tile_hovered_pos)) {
            return false;
        }

        HurtTile(context.world->tilemap, tile_hovered_pos, damage);
#endif

        return true;
    }

    constexpr zcl::t_static_array<t_item_type_use_func, ekm_item_type_id_cnt> k_item_type_use_funcs = {{
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

#if 0
    void ProcessItemUsage(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        if (world->player_entity.item_use_time > 0) {
            world->player_entity.item_use_time--;
        } else {
            const t_inventory_slot hotbar_slot_selected = InventoryGet(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index);

            if (hotbar_slot_selected.quantity > 0) {
                const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

                const zcl::t_b8 item_use = g_item_types[item_type_id].use_hold ? zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left) : zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left);

                if (item_use) {
                    const zcl::t_b8 item_use_success = k_item_type_use_funcs[item_type_id]({
                        .world = world,
                        .cursor_pos = zgl::CursorGetPos(input_state),
                        .screen_size = screen_size,
                        .temp_arena = temp_arena,
                    });

                    if (item_use_success) {
                        if (g_item_types[item_type_id].use_consume) {
                            InventoryRemoveAt(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index, 1);
                        }

                        world->player_entity.item_use_time = g_item_types[hotbar_slot_selected.item_type_id].use_time;
                    }
                }
            }
        }
    }
#endif
}
