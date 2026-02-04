#include "world_private.h"

#include "inventories.h"
#include "tilemaps.h"
#include "camera.h"

namespace world {
    // Returns true iff a slot is hovered.
    [[nodiscard]] static zcl::t_b8 GetPlayerInventoryHoveredSlotPos(const t_inventory *const inventory, const zcl::t_b8 inventory_open, const zcl::t_v2 cursor_position, zcl::t_v2_i *const o_slot_pos) {
        const zcl::t_v2 cursor_pos_rel_to_inventory_top_left = cursor_position - k_ui_player_inventory_offs_top_left;

        const zcl::t_v2_i inventory_size = InventoryGetSize(inventory);
        const zcl::t_v2 inventory_size_in_pixels = k_ui_player_inventory_slot_distance * zcl::V2IToF(inventory_size);

        if (cursor_pos_rel_to_inventory_top_left.x >= 0.0f && cursor_pos_rel_to_inventory_top_left.y >= 0.0f && cursor_pos_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_pos_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
            const zcl::t_v2_i slot_pos = {
                static_cast<zcl::t_i32>(floor(cursor_pos_rel_to_inventory_top_left.x / k_ui_player_inventory_slot_distance)),
                static_cast<zcl::t_i32>(floor(cursor_pos_rel_to_inventory_top_left.y / k_ui_player_inventory_slot_distance)),
            };

            if (slot_pos.y == 0 || inventory_open) {
                const zcl::t_v2 cursor_pos_rel_to_slot_region = cursor_pos_rel_to_inventory_top_left - (zcl::V2IToF(slot_pos) * k_ui_player_inventory_slot_distance);

                if (cursor_pos_rel_to_slot_region.x < k_ui_player_inventory_slot_size && cursor_pos_rel_to_slot_region.y < k_ui_player_inventory_slot_size) {
                    *o_slot_pos = slot_pos;
                    return true;
                }
            }
        }

        return false;
    }

    static zcl::t_rect_f CalcPlayerInventorySlotRect(const zcl::t_v2_i slot_pos) {
        const zcl::t_v2 slot_pos_screen = k_ui_player_inventory_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_ui_player_inventory_slot_distance);
        const zcl::t_v2 slot_size = {k_ui_player_inventory_slot_size, k_ui_player_inventory_slot_size};

        return zcl::RectCreateF(slot_pos_screen, slot_size);
    }

    void ProcessPlayerInventoryInteraction(t_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state) {
        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
            ui->player_inventory_open = !ui->player_inventory_open;
        }

        if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
            zcl::t_v2_i slot_hovered_pos;

            if (GetPlayerInventoryHoveredSlotPos(player_inventory, ui->player_inventory_open, cursor_pos, &slot_hovered_pos)) {
                const auto slot = InventoryGet(player_inventory, slot_hovered_pos);

                if (ui->cursor_held_quantity == 0) {
                    ui->cursor_held_item_type_id = slot.item_type_id;
                    ui->cursor_held_quantity = slot.quantity;

                    InventoryRemoveAt(player_inventory, slot_hovered_pos, slot.quantity);
                } else {
                    if (slot.quantity == 0) {
                        InventoryAddAt(player_inventory, slot_hovered_pos, ui->cursor_held_item_type_id, ui->cursor_held_quantity);
                        ui->cursor_held_quantity = 0;
                    }
                }
            }
        }
    }

    void RenderPopUps(const zgl::t_rendering_context rc, const t_pop_up_manager *const pop_ups, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena) {
        ZCL_BITSET_WALK_ALL_SET (pop_ups->activity, i) {
            const auto pop_up = &pop_ups->buf[i];

            const zcl::t_f32 life_perc = 1.0f - (static_cast<zcl::t_f32>(pop_up->death_time) / k_pop_up_death_duration);

            zgl::RendererSubmitStr(rc, {{pop_up->str_bytes.raw, pop_up->str_byte_cnt}}, *GetFont(assets, pop_up->font_id), CameraToScreenPos(pop_up->pos, camera, rc.screen_size), zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, life_perc), temp_arena, zcl::k_origin_center, 0.0f, {life_perc, life_perc});
        }
    }

    void RenderTileHighlight(const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_camera *const camera) {
        const zcl::t_v2_i tile_hovered_pos = ScreenToTilePos(cursor_pos, rc.screen_size, camera);
        const zcl::t_v2 tile_hovered_pos_world = zcl::V2IToF(tile_hovered_pos) * k_tile_size;

        const zcl::t_rect_f rect = zcl::RectCreateF(CameraToScreenPos(tile_hovered_pos_world, camera, rc.screen_size), zcl::t_v2{k_tile_size, k_tile_size} * CameraGetScale(camera));

        zgl::RendererSubmitRect(rc, rect, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, k_ui_tile_highlight_alpha));
    }

    static zcl::t_str_mut DetermineCursorHoverStr(const zcl::t_v2 cursor_pos, const t_inventory *const player_inventory, const zcl::t_b8 player_inventory_open, const t_npc_manager *const npc_manager, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
        constexpr zcl::t_i32 k_str_len_limit = 32;

        {
            zcl::t_v2_i player_inventory_slot_hovered_pos;

            if (GetPlayerInventoryHoveredSlotPos(player_inventory, player_inventory_open, cursor_pos, &player_inventory_slot_hovered_pos)) {
                const t_inventory_slot player_inventory_slot_hovered = InventoryGet(player_inventory, player_inventory_slot_hovered_pos);

                if (player_inventory_slot_hovered.quantity > 0) {
                    return zcl::StrClone(g_item_types[player_inventory_slot_hovered.item_type_id].name, arena);
                }
            }
        }

        ZCL_BITSET_WALK_ALL_SET (npc_manager->activity, i) {
            const auto npc = &npc_manager->buf[i];
            const zcl::t_rect_f npc_collider = GetNPCCollider(npc->pos, npc->type_id);
            const zcl::t_rect_f npc_collider_screen = zcl::RectCreateF(CameraToScreenPos(zcl::RectGetPos(npc_collider), camera, screen_size), zcl::RectGetSize(npc_collider) * CameraGetScale(camera));

            if (zcl::CheckPointInRect(cursor_pos, npc_collider_screen)) {
                return zcl::StrClone(g_npc_types[npc->type_id].name, arena);
            }
        }

        return {};
    }

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

    void RenderPlayerInventory(const zgl::t_rendering_context rc, const t_ui *const ui, const t_player_meta *const player_meta, const t_assets *const assets, zcl::t_arena *const temp_arena) {
        const zcl::t_v2_i inventory_size = InventoryGetSize(player_meta->inventory);

        const zcl::t_i32 slot_cnt_y = ui->player_inventory_open ? inventory_size.y : 1;

        for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
            for (zcl::t_i32 slot_x = 0; slot_x < inventory_size.x; slot_x++) {
                const zcl::t_i32 slot_index = (slot_y * inventory_size.x) + slot_x;

                const auto slot = InventoryGet(player_meta->inventory, {slot_x, slot_y});

                const auto ui_slot_rect = CalcPlayerInventorySlotRect({slot_x, slot_y});

                const auto ui_slot_color = slot_y == 0 && player_meta->inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
                ZCL_ASSERT(ui_slot_color.a == 1.0f);

                zgl::RendererSubmitRect(rc, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_inventory_slot_bg_alpha));
                zgl::RendererSubmitRectOutlineOpaque(rc, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

                if (slot.quantity > 0) {
                    UIRenderItem(slot.item_type_id, slot.quantity, rc, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
                }
            }
        }
    }

    void RenderPlayerHealth(const zgl::t_rendering_context rc, const zcl::t_i32 health, const zcl::t_i32 health_limit) {
        ZCL_ASSERT(health >= 0 && health <= health_limit);

        const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(rc.screen_size.x) - k_ui_player_health_bar_offs_top_right.x - k_ui_player_health_bar_size.x, k_ui_player_health_bar_offs_top_right.y};
        const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_ui_player_health_bar_size);

        zgl::RendererSubmitRectOutlineOpaque(rc, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);

        zgl::RendererSubmitRect(rc, zcl::RectCreateF(health_bar_rect.x, health_bar_rect.y, health_bar_rect.width * (static_cast<zcl::t_f32>(health) / health_limit), health_bar_rect.height), zcl::k_color_white);
    }

    void RenderCursorHeldItem(const t_ui *const ui, const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
        if (ui->cursor_held_quantity > 0) {
            UIRenderItem(ui->cursor_held_item_type_id, ui->cursor_held_quantity, rc, cursor_pos, assets, temp_arena);
        }
    }

    void RenderCursorHoverStr(const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_inventory *const player_inventory, const zcl::t_b8 player_inventory_open, const t_npc_manager *const npc_manager, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena) {
        const auto cursor_hover_str = DetermineCursorHoverStr(cursor_pos, player_inventory, player_inventory_open, npc_manager, camera, rc.screen_size, temp_arena);

        if (!zcl::StrCheckEmpty(cursor_hover_str)) {
            zgl::RendererSubmitStr(rc, cursor_hover_str, *GetFont(assets, ek_font_id_eb_garamond_32), cursor_pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
        }
    }
}
