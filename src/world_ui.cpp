#include "world_private.h"

#include "items.h"
#include "inventory.h"

constexpr zcl::t_v2 k_player_inventory_offs_top_left = {48.0f, 48.0f};
constexpr zcl::t_f32 k_player_inventory_slot_size = 48.0f;
constexpr zcl::t_f32 k_player_inventory_slot_distance = 64.0f;
constexpr zcl::t_f32 k_player_inventory_slot_bg_alpha = 0.2f;

constexpr zcl::t_v2 k_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_player_health_bar_size = {240.0f, 24.0f};

constexpr zcl::t_i32 k_player_inventory_slot_cnt_x = 7;
static_assert(k_player_inventory_slot_cnt_x <= 9, "You need to be able to map numeric keys to hotbar slots.");

constexpr zcl::t_i32 k_player_inventory_slot_cnt_y = 4;

static_assert(k_player_inventory_slot_cnt_x * k_player_inventory_slot_cnt_y == k_player_inventory_slot_cnt);

struct t_world_ui {
    zcl::t_i32 inventory_open;
    zcl::t_i32 inventory_hotbar_slot_selected_index;

    t_item_type_id cursor_held_item_type_id;
    zcl::t_i32 cursor_held_quantity;
};

static void RenderItem(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rendering_context, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    SpriteRender(g_item_types[item_type_id].icon_sprite_id, rendering_context, assets, pos, zcl::k_origin_center, 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rendering_context, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *GetFont(assets, ek_font_id_eb_garamond_24), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

// Returns -1 if no slot is hovered.
static zcl::t_i32 InventoryGetHoveredSlotIndex(const zcl::t_v2 cursor_position, const zcl::t_b8 inventory_open) {
    const zcl::t_v2 cursor_position_rel_to_inventory_top_left = cursor_position - k_player_inventory_offs_top_left;
    const zcl::t_v2 inventory_size_in_pixels = k_player_inventory_slot_distance * zcl::t_v2{k_player_inventory_slot_cnt_x, k_player_inventory_slot_cnt_y};

    if (cursor_position_rel_to_inventory_top_left.x >= 0.0f && cursor_position_rel_to_inventory_top_left.y >= 0.0f && cursor_position_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_position_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_position_in_grid = {
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.x / k_player_inventory_slot_distance)),
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.y / k_player_inventory_slot_distance)),
        };

        if (slot_position_in_grid.y == 0 || inventory_open) {
            const zcl::t_v2 cursor_position_rel_to_slot_region = cursor_position_rel_to_inventory_top_left - (zcl::V2IToF(slot_position_in_grid) * k_player_inventory_slot_distance);

            if (cursor_position_rel_to_slot_region.x < k_player_inventory_slot_size && cursor_position_rel_to_slot_region.y < k_player_inventory_slot_size) {
                return (slot_position_in_grid.y * k_player_inventory_slot_cnt_x) + slot_position_in_grid.x;
            }
        }
    }

    return -1;
}

static zcl::t_rect_f InventoryCalcSlotRect(const zcl::t_i32 slot_index) {
    ZCL_ASSERT(slot_index >= 0 && slot_index < k_player_inventory_slot_cnt);

    const zcl::t_v2_i slot_pos = {slot_index % k_player_inventory_slot_cnt_x, slot_index / k_player_inventory_slot_cnt_x};
    const zcl::t_v2 ui_slot_pos = k_player_inventory_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_player_inventory_slot_distance);
    const zcl::t_v2 ui_slot_size = {k_player_inventory_slot_size, k_player_inventory_slot_size};

    return zcl::RectCreateF(ui_slot_pos, ui_slot_size);
}

t_world_ui *WorldUICreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_world_ui>(arena);
}

void WorldUITick(t_world_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state) {
    // @todo: Scroll support.

    for (zcl::t_i32 i = 0; i < k_player_inventory_slot_cnt_x; i++) {
        if (zgl::KeyCheckPressed(input_state, static_cast<zgl::t_key_code>(zgl::ek_key_code_1 + i))) {
            ui->inventory_hotbar_slot_selected_index = i;
            break;
        }
    }

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        ui->inventory_open = !ui->inventory_open;
    }

    if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
        const zcl::t_i32 slot_hovered_index = InventoryGetHoveredSlotIndex(zgl::CursorGetPos(input_state), ui->inventory_open);

        if (slot_hovered_index != -1) {
            const auto slot = InventoryGet(player_inventory, slot_hovered_index);

            if (ui->cursor_held_quantity == 0) {
                ui->cursor_held_item_type_id = slot.item_type_id;
                ui->cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(player_inventory, slot_hovered_index, slot.quantity);
            } else {
                if (slot.quantity == 0) {
                    InventoryAddAt(player_inventory, slot_hovered_index, ui->cursor_held_item_type_id, ui->cursor_held_quantity);
                    ui->cursor_held_quantity = 0;
                }
            }
        }
    }
}

void WorldUIRender(const t_world_ui *const ui, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const auto backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

#if 0
    //
    // Inventory
    //
    const zcl::t_i32 slot_cnt_y = ui->inventory_open ? k_player_inventory_slot_cnt_y : 1;

    for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
        for (zcl::t_i32 slot_x = 0; slot_x < k_player_inventory_slot_cnt_x; slot_x++) {
            const zcl::t_i32 slot_index = (slot_y * k_player_inventory_slot_cnt_x) + slot_x;

            const auto slot = InventoryGet(ui->inventory, slot_index);

            const auto ui_slot_rect = PlayerInventoryUISlotCalcRect(slot_index);

            const auto ui_slot_color = slot_y == 0 && ui->inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
            ZCL_ASSERT(ui_slot_color.a == 1.0f);

            zgl::RendererSubmitRect(rendering_context, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_player_inventory_slot_bg_alpha));
            zgl::RendererSubmitRectOutlineOpaque(rendering_context, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

            if (slot.quantity > 0) {
                RenderItemUI(slot.item_type_id, slot.quantity, rendering_context, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
            }
        }
    }

    //
    // Health
    //
    const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(backbuffer_size.x) - k_player_health_bar_offs_top_right.x - k_player_health_bar_size.x, k_player_health_bar_offs_top_right.y};
    const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_player_health_bar_size);

    zgl::RendererSubmitRectOutlineOpaque(rendering_context, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);

    //
    // Cursor Held
    //
    if (ui->cursor_held_quantity > 0) {
        RenderItemUI(ui->cursor_held_item_type_id, ui->cursor_held_quantity, rendering_context, zgl::CursorGetPos(input_state), assets, temp_arena);
    }
#endif
}
