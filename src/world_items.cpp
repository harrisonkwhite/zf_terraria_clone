#include "world_private.h"

#include "inventory.h"

void ProcessItemUsage(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    if (world->player_entity.item_use_time > 0) {
        world->player_entity.item_use_time--;
    } else {
        if (zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left)) {
            const t_inventory_slot hotbar_slot_selected = InventoryGet(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index);
            const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

            if (hotbar_slot_selected.quantity > 0) {
            }
        }
    }
}

#if 0
struct t_item_type_use_func_context {
    t_world *world;
    zcl::t_v2 cursor_pos;
    zcl::t_v2_i screen_size;
    zcl::t_arena *temp_arena;
};

using t_item_type_use_func = zcl::t_b8 (*)(const t_item_type_use_func_context &context);

// Data relevant to the item type only in the context of the world phase.
struct t_item_type_info_world {
    t_item_type_use_func use_func; // Called when the item is used. Should return true iff the item was successfully used (this info is needed to determine whether to consume the item for example).
};

static zcl::t_b8 Test(const t_item_type_use_func_context &context, const t_tile_type_id tile_type_id) {
    const zcl::t_v2_i tile_hovered_pos = zcl::V2FToI(ScreenToCameraPos(context.cursor_pos, context.screen_size, context.world->camera) / k_tile_size);

    if (TilemapCheck(context.world->tilemap, tile_hovered_pos)) {
        return false;
    }

    TilemapAdd(context.world->tilemap, tile_hovered_pos, tile_type_id);

    return true;
}

static const zcl::t_static_array<t_item_type_info_world, ekm_item_type_id_cnt> g_item_type_infos_world = {{
    {
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_func = [](const t_item_type_use_func_context &context) {
            return Test(context, ek_tile_type_id_dirt);
        },
    },
    {
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_func = [](const t_item_type_use_func_context &context) {
            return Test(context, ek_tile_type_id_stone);
        },
    },
    {
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_func = [](const t_item_type_use_func_context &context) {
            return Test(context, ek_tile_type_id_grass);
        },
    },
}}; // @todo: Some way to static assert that all array elements have been set! This is for any static array!

void Yeah(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    if (world->player_entity.item_use_time > 0) {
        world->player_entity.item_use_time--;
    } else {
        if (zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left)) {
            const t_inventory_slot hotbar_slot_selected = InventoryGet(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index);
            const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

            if (hotbar_slot_selected.quantity > 0) {
                if (g_item_type_infos_world[item_type_id].use_func) { // @note: Maybe it should be mandatory?
                    const t_item_type_use_func_context item_use_func_context = {
                        .world = world,
                        .cursor_pos = cursor_pos,
                        .screen_size = screen_size,
                        .temp_arena = temp_arena,
                    };

                    const zcl::t_b8 item_used = g_item_type_infos_world[item_type_id].use_func(item_use_func_context);

                    if (item_used) {
                        if (g_item_type_infos_world[item_type_id].use_consume) {
                            InventoryRemoveAt(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index, 1);
                        }

                        world->player_entity.item_use_time = g_item_type_infos_world[hotbar_slot_selected.item_type_id].use_time;
                    }
                }
            }
        }
    }
}
#endif
