#include "world_private.h"

#include "inventory.h"
#include "camera.h"

constexpr zcl::t_f32 k_ui_tile_highlight_alpha = 0.6f;

constexpr zcl::t_v2 k_ui_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_ui_player_health_bar_size = {240.0f, 24.0f};

constexpr zcl::t_v2 k_ui_player_inventory_offs_top_left = {48.0f, 48.0f};

constexpr zcl::t_f32 k_ui_player_inventory_slot_size = 48.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_distance = 64.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_bg_alpha = 0.2f;

static void UIRenderItem(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    SpriteRender(g_item_types[item_type_id].icon_sprite_id, rc, assets, pos, zcl::k_origin_center, 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rc, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *GetFont(assets, ek_font_id_eb_garamond_24), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

// Returns -1 if no slot is hovered.
static zcl::t_i32 UIPlayerInventoryGetHoveredSlotIndex(const zcl::t_v2 cursor_position, const zcl::t_b8 inventory_open) {
    const zcl::t_v2 cursor_position_rel_to_inventory_top_left = cursor_position - k_ui_player_inventory_offs_top_left;
    const zcl::t_v2 inventory_size_in_pixels = k_ui_player_inventory_slot_distance * zcl::t_v2{k_player_inventory_width, k_player_inventory_height};

    if (cursor_position_rel_to_inventory_top_left.x >= 0.0f && cursor_position_rel_to_inventory_top_left.y >= 0.0f && cursor_position_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_position_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_position_in_grid = {
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.x / k_ui_player_inventory_slot_distance)),
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.y / k_ui_player_inventory_slot_distance)),
        };

        if (slot_position_in_grid.y == 0 || inventory_open) {
            const zcl::t_v2 cursor_position_rel_to_slot_region = cursor_position_rel_to_inventory_top_left - (zcl::V2IToF(slot_position_in_grid) * k_ui_player_inventory_slot_distance);

            if (cursor_position_rel_to_slot_region.x < k_ui_player_inventory_slot_size && cursor_position_rel_to_slot_region.y < k_ui_player_inventory_slot_size) {
                return (slot_position_in_grid.y * k_player_inventory_width) + slot_position_in_grid.x;
            }
        }
    }

    return -1;
}

static zcl::t_rect_f UIPlayerInventoryCalcSlotRect(const zcl::t_v2_i slot_pos) {
    ZCL_ASSERT(slot_pos.x >= 0 && slot_pos.y >= 0 && slot_pos.x < k_player_inventory_width && slot_pos.y < k_player_inventory_height);

    const zcl::t_v2 slot_pos_screen = k_ui_player_inventory_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_ui_player_inventory_slot_distance);

    const zcl::t_v2 slot_size = {k_ui_player_inventory_slot_size, k_ui_player_inventory_slot_size};

    return zcl::RectCreateF(slot_pos_screen, slot_size);
}

void UITick(t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    // ----------------------------------------
    // Player Inventory

    t_inventory *const player_inventory = PlayerGetInventory(world->player_meta);

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        world->ui.player_inventory_open = !world->ui.player_inventory_open;
    }

    if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
        const zcl::t_i32 slot_hovered_index = UIPlayerInventoryGetHoveredSlotIndex(cursor_pos, world->ui.player_inventory_open);

        if (slot_hovered_index != -1) {
            const auto slot = InventoryGet(player_inventory, slot_hovered_index);

            if (world->ui.cursor_held_quantity == 0) {
                world->ui.cursor_held_item_type_id = slot.item_type_id;
                world->ui.cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(player_inventory, slot_hovered_index, slot.quantity);
            } else {
                if (slot.quantity == 0) {
                    InventoryAddAt(player_inventory, slot_hovered_index, world->ui.cursor_held_item_type_id, world->ui.cursor_held_quantity);
                    world->ui.cursor_held_quantity = 0;
                }
            }
        }
    }

    // ------------------------------
}

void UIRender(const t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    // ----------------------------------------
    // Tile Highlight

    {
        const zcl::t_v2_i tile_hovered_pos = zcl::V2FToI(ScreenToCameraPos(cursor_pos, rc.screen_size, world->camera) / k_tile_size);
        const zcl::t_v2 tile_hovered_pos_world = zcl::V2IToF(tile_hovered_pos) * k_tile_size;

        const zcl::t_rect_f rect = zcl::RectCreateF(CameraToScreenPos(tile_hovered_pos_world, world->camera, rc.screen_size), zcl::t_v2{k_tile_size, k_tile_size} * CameraGetScale(world->camera));

        zgl::RendererSubmitRect(rc, rect, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, k_ui_tile_highlight_alpha));
    }

    // ------------------------------

    // ----------------------------------------
    // Pop-Ups

    ZCL_BITSET_WALK_ALL_SET (world->pop_ups.activity, i) {
        const auto pop_up = &world->pop_ups.buf[i];

        const zcl::t_f32 life_perc = 1.0f - (static_cast<zcl::t_f32>(pop_up->death_time) / k_pop_up_death_time_limit);

        zgl::RendererSubmitStr(rc, {{pop_up->str_bytes.raw, pop_up->str_byte_cnt}}, *GetFont(assets, pop_up->font_id), CameraToScreenPos(pop_up->pos, world->camera, rc.screen_size), zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, life_perc), temp_arena, zcl::k_origin_center, 0.0f, {life_perc, life_perc});
    }

    // ------------------------------

#if 0
    // ----------------------------------------
    // Inventory

    {
        const zcl::t_i32 slot_cnt_y = world->ui.player_inventory_open ? k_ui_player_inventory_slot_cnt_y : 1;

        for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
            for (zcl::t_i32 slot_x = 0; slot_x < k_ui_player_inventory_slot_cnt_x; slot_x++) {
                const zcl::t_i32 slot_index = (slot_y * k_ui_player_inventory_slot_cnt_x) + slot_x;

                const auto slot = InventoryGet(world->player_inventory, slot_index);

                const auto ui_slot_rect = UIPlayerInventoryCalcSlotRect(slot_index);

                const auto ui_slot_color = slot_y == 0 && world->ui.player_inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
                ZCL_ASSERT(ui_slot_color.a == 1.0f);

                zgl::RendererSubmitRect(rc, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_inventory_slot_bg_alpha));
                zgl::RendererSubmitRectOutlineOpaque(rc, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

                if (slot.quantity > 0) {
                    UIRenderItem(slot.item_type_id, slot.quantity, rc, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
                }
            }
        }
    }

    // ------------------------------
#endif

#if 0
    // --------------------------------------------------
    // Health

    {
        const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(rc.screen_size.x) - k_ui_player_health_bar_offs_top_right.x - k_ui_player_health_bar_size.x, k_ui_player_health_bar_offs_top_right.y};
        const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_ui_player_health_bar_size);

        zgl::RendererSubmitRectOutlineOpaque(rc, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);
    }

    // ------------------------------

    // --------------------------------------------------
    // Cursor Held

    if (world->ui.cursor_held_quantity > 0) {
        UIRenderItem(world->ui.cursor_held_item_type_id, world->ui.cursor_held_quantity, rc, cursor_pos, assets, temp_arena);
    }

    // ------------------------------
#endif
}
