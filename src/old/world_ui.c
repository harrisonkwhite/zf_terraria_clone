#include "game.h"

#include <stdio.h>

#define PLAYER_HP_BAR_WIDTH 240.0f
#define PLAYER_HP_BAR_HEIGHT 16.0f
#define PLAYER_HP_POS_PERC (s_v2){0.95f, 0.075f}

#define PLAYER_INVENTORY_POS_PERC (s_v2){0.05f, 0.075f}
#define PLAYER_INVENTORY_BG_ALPHA 0.6f
#define PLAYER_INVENTORY_SLOT_GAP 64.0f
#define PLAYER_INVENTORY_SLOT_SIZE 48.0f
#define PLAYER_INVENTORY_SLOT_BG_ALPHA 0.4f

static s_v2 PlayerInventorySlotPos(const t_s32 r, const t_s32 c, const s_v2_s32 ui_size) {
    assert(r >= 0 && r < PLAYER_INVENTORY_ROW_CNT);
    assert(c >= 0 && c < PLAYER_INVENTORY_COLUMN_CNT);
    assert(ui_size.x > 0 && ui_size.y > 0);

    const s_v2 top_left = {ui_size.x * PLAYER_INVENTORY_POS_PERC.x, ui_size.y * PLAYER_INVENTORY_POS_PERC.y};

    return (s_v2){
        top_left.x + (PLAYER_INVENTORY_SLOT_GAP * c),
        top_left.y + (PLAYER_INVENTORY_SLOT_GAP * r)
    };
}

static void UpdatePlayerInventoryHotbarSlotSelected(t_s32* const hotbar_slot_selected, const s_input_context* const input_context) {
    assert(hotbar_slot_selected);
    assert(*hotbar_slot_selected >= 0 && *hotbar_slot_selected < PLAYER_INVENTORY_COLUMN_CNT);

    for (t_s32 i = 0; i < PLAYER_INVENTORY_COLUMN_CNT; i++) {
        if (IsKeyPressed(input_context, ek_key_code_1 + i)) {
            *hotbar_slot_selected = i;
            break;
        }
    }

    if (input_context->events->mouse_scroll_state == ek_mouse_scroll_state_down) {
        (*hotbar_slot_selected)++;
        (*hotbar_slot_selected) %= PLAYER_INVENTORY_COLUMN_CNT;
    } else if (input_context->events->mouse_scroll_state == ek_mouse_scroll_state_up) {
        (*hotbar_slot_selected)--;

        if (*hotbar_slot_selected < 0) {
            *hotbar_slot_selected += PLAYER_INVENTORY_COLUMN_CNT;
        }
    }

    assert(*hotbar_slot_selected >= 0 && *hotbar_slot_selected < PLAYER_INVENTORY_COLUMN_CNT);
}

static void ProcPlayerInventoryOpenState(s_world* const world, const s_input_context* const input_context, const s_v2_s32 window_size) {
    assert(world->player_inv_open);

    const s_v2_s32 ui_size = UISize(window_size);
    const s_v2 cursor_ui_pos = DisplayToUIPos(input_context->state->mouse_pos, window_size);

    for (t_s32 r = 0; r < PLAYER_INVENTORY_ROW_CNT; r++) {
        for (t_s32 c = 0; c < PLAYER_INVENTORY_COLUMN_CNT; c++) {
            s_inventory_slot* const slot = &world->player_inv_slots[r][c];

            const s_v2 slot_pos = PlayerInventorySlotPos(r, c, ui_size);

            const s_rect slot_collider = {
                slot_pos.x,
                slot_pos.y,
                PLAYER_INVENTORY_SLOT_SIZE,
                PLAYER_INVENTORY_SLOT_SIZE
            };

            if (IsPointInRect(cursor_ui_pos, slot_collider)) {
                // Handle slot click event.
                const bool clicked = IsMouseButtonPressed(input_context, ek_mouse_button_code_left);

                if (clicked) {
                    if (slot->quantity > 0 && world->mouse_item_held_quantity > 0 && slot->item_type == world->mouse_item_held_type) {
                        const t_s32 to_add = MIN(world->mouse_item_held_quantity, ITEM_QUANTITY_LIMIT - slot->quantity);

                        if (to_add == 0) {
                            const e_item_type item_type_temp = slot->item_type;
                            const t_s32 quantity_temp = slot->quantity;

                            slot->item_type = world->mouse_item_held_type;
                            slot->quantity = world->mouse_item_held_quantity;

                            world->mouse_item_held_type = item_type_temp;
                            world->mouse_item_held_quantity = quantity_temp;
                        } else {
                            slot->quantity += to_add;
                            world->mouse_item_held_quantity -= to_add;
                        }
                    } else {
                        const e_item_type item_type_temp = slot->item_type;
                        const t_s32 quantity_temp = slot->quantity;

                        slot->item_type = world->mouse_item_held_type;
                        slot->quantity = world->mouse_item_held_quantity;

                        world->mouse_item_held_type = item_type_temp;
                        world->mouse_item_held_quantity = quantity_temp;
                    }
                }

                break;
            }
        }
    }
}

static void WriteItemNameStr(char* const str_buf, const t_s32 str_buf_size, const e_item_type item_type, const t_s32 quantity) {
    assert(str_buf);
    assert(str_buf_size > 0);
    assert(quantity >= 1);

    if (quantity == 1) {
        snprintf(str_buf, str_buf_size, "%s", g_item_type_infos[item_type].name.buf_raw);
    } else {
        snprintf(str_buf, str_buf_size, "%s (%d)", g_item_type_infos[item_type].name.buf_raw, quantity);
    }
}

static void LoadMouseHoverStr(t_mouse_hover_str_buf* const hover_str_buf, const s_v2 mouse_pos, const s_world* const world, const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 mouse_cam_pos = DisplayToCameraPos(mouse_pos, &world->cam, window_size);
    const s_v2 mouse_ui_pos = DisplayToUIPos(mouse_pos, window_size);

    // TODO: The sequencing of the below should really correspond to draw layering. Maybe add some layer depth variable?

    assert(hover_str_buf && IS_ZERO(*hover_str_buf));

    if (world->player_inv_open) {
        for (t_s32 r = 0; r < PLAYER_INVENTORY_ROW_CNT; r++) {
            for (t_s32 c = 0; c < PLAYER_INVENTORY_COLUMN_CNT; c++) {
                const s_inventory_slot* const slot = &world->player_inv_slots[r][c];

                if (slot->quantity == 0) {
                    continue;
                }

                const s_v2 slot_pos = PlayerInventorySlotPos(r, c, UISize(window_size));

                const s_rect slot_collider = {
                    slot_pos.x,
                    slot_pos.y,
                    PLAYER_INVENTORY_SLOT_SIZE,
                    PLAYER_INVENTORY_SLOT_SIZE
                };

                if (IsPointInRect(mouse_ui_pos, slot_collider)) {
                    WriteItemNameStr(*hover_str_buf, sizeof(*hover_str_buf), slot->item_type, slot->quantity);
                    break;
                }
            }
        }
    } else {
        //
        // Item Drops
        //
        for (t_s32 i = 0; i < world->item_drop_active_cnt; i++) {
            const s_item_drop* const drop = &world->item_drops[i];
            const s_rect drop_collider = ItemDropCollider(drop->pos, drop->item_type);

            if (IsPointInRect(mouse_cam_pos, drop_collider)) {
                WriteItemNameStr(*hover_str_buf, sizeof(*hover_str_buf), drop->item_type, drop->quantity);
                break;
            }
        }

        //
        // NPC Hovering
        //
        for (t_s32 i = 0; i < NPC_LIMIT; i++) {
            if (!IsNPCActivityBitSet(&world->npcs.activity, i)) {
                continue;
            }

            const s_npc* const npc = &world->npcs.buf[i];
            const s_npc_type* const npc_type = &g_npc_types[npc->type];
            const s_rect npc_collider = NPCCollider(npc->pos, npc->type);

            if (IsPointInRect(mouse_cam_pos, npc_collider)) {
                snprintf(*hover_str_buf, sizeof(*hover_str_buf), "%s (%d/%d)", npc_type->name.buf_raw, npc->hp, npc_type->hp_max);
                break;
            }
        }
    }
}

void UpdateWorldUI(s_world* const world, const s_input_context* const input_context, const s_v2_s32 window_size) {
    if (!world->player.killed) {
        UpdatePlayerInventoryHotbarSlotSelected(&world->player_inv_hotbar_slot_selected, input_context);

        if (IsKeyPressed(input_context, ek_key_code_escape)) {
            world->player_inv_open = !world->player_inv_open;
        }
    } else {
        world->player_inv_open = false;
    }

    if (world->player_inv_open) {
        ProcPlayerInventoryOpenState(world, input_context, window_size);
    }

    ZERO_OUT(world->mouse_hover_str);
    LoadMouseHoverStr(&world->mouse_hover_str, input_context->state->mouse_pos, world, window_size);

    // Update popup text.
    for (t_s32 i = 0; i < POPUP_TEXT_LIMIT; i++) {
        s_popup_text* const popup = &world->popup_texts[i];

        assert(popup->alpha >= 0.0f && popup->alpha <= 1.0f);

        if (popup->alpha <= POPUP_TEXT_INACTIVITY_ALPHA_THRESH) {
            continue;
        }

        if (fabs(popup->vel_y) <= POPUP_TEXT_FADE_VEL_Y_ABS_THRESH) {
            popup->alpha *= POPUP_TEXT_ALPHA_MULT;
        }

        popup->pos.y += popup->vel_y;

        popup->vel_y *= POPUP_TEXT_VEL_Y_MULT;
    }
}

static void RenderTileHighlight(const s_world* const world, const s_rendering_context* const rendering_context, const s_v2 mouse_pos) {
    const s_inventory_slot* const active_slot = &world->player_inv_slots[0][world->player_inv_hotbar_slot_selected];

    if (!world->player_inv_open && active_slot->quantity > 0) {
        const s_v2 mouse_cam_pos = DisplayToCameraPos(mouse_pos, &world->cam, rendering_context->window_size);
        const s_v2_s32 mouse_tile_pos = CameraToTilePos(mouse_cam_pos);

        const s_item_type_info* const active_item = &g_item_type_infos[active_slot->item_type];

        if ((active_item->use_type == ek_item_use_type_tile_place || active_item->use_type == ek_item_use_type_tile_hurt) && IsItemUsable(active_slot->item_type, world, mouse_tile_pos)) {
            const s_v2 mouse_cam_pos_snapped_to_tilemap = {mouse_tile_pos.x * TILE_SIZE, mouse_tile_pos.y * TILE_SIZE};

            const s_v2 highlight_pos = CameraToUIPos(mouse_cam_pos_snapped_to_tilemap, &world->cam, rendering_context->window_size);
            const t_r32 highlight_size = (t_r32)(TILE_SIZE * world->cam.scale) / UIScale(rendering_context->window_size);
            const s_rect highlight_rect = {
                .x = highlight_pos.x,
                .y = highlight_pos.y,
                .width = highlight_size,
                .height = highlight_size
            };
            RenderRect(rendering_context, highlight_rect, (u_v4){1.0f, 1.0f, 1.0f, TILE_HIGHLIGHT_ALPHA});
        }
    }
}

static bool RenderInventorySlot(const s_rendering_context* const rendering_context, const s_inventory_slot slot, const s_v2 pos, const u_v4 outline_color, const s_texture_group* const textures, const s_font_group* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_rect slot_rect = {
        pos.x, pos.y,
        PLAYER_INVENTORY_SLOT_SIZE, PLAYER_INVENTORY_SLOT_SIZE
    };

    // Render the slot box.
    RenderRectWithOutline(rendering_context, slot_rect, (u_v4){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_SLOT_BG_ALPHA}, outline_color, 1.0f);

    // Render the item icon.
    if (slot.quantity > 0) {
        RenderSprite(rendering_context, g_item_type_infos[slot.item_type].icon_spr, textures, RectCenter(slot_rect), (s_v2){0.5f, 0.5f}, (s_v2){1.0f, 1.0f}, 0.0f, WHITE);
    }

    return true;
}

static bool RenderPlayerInventory(const s_rendering_context* const rendering_context, const s_world* const world, const s_texture_group* const textures, const s_font_group* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_v2_s32 ui_size = UISize(rendering_context->window_size);

    if (world->player_inv_open) {
        // Draw a backdrop.
        const s_v2_s32 ui_size = UISize(rendering_context->window_size);
        const s_rect bg_rect = {0.0f, 0.0f, ui_size.x, ui_size.y};
        RenderRect(rendering_context, bg_rect, (u_v4){0.0f, 0.0f, 0.0f, PLAYER_INVENTORY_BG_ALPHA});
    } else {
        // Render current item name.
        const s_inventory_slot* const slot = &world->player_inv_slots[0][world->player_inv_hotbar_slot_selected];

        if (slot->quantity > 0) {
            char name_buf[32];
            WriteItemNameStr(name_buf, sizeof(name_buf), slot->item_type, slot->quantity);

            const s_v2 name_pos = {
                ui_size.x * PLAYER_INVENTORY_POS_PERC.x,
                (ui_size.y * PLAYER_INVENTORY_POS_PERC.y) + PLAYER_INVENTORY_SLOT_SIZE + 8.0f
            };

            if (!RenderStr(rendering_context, (s_char_array_view)ARRAY_FROM_STATIC(name_buf), fonts, ek_font_eb_garamond_24, name_pos, ALIGNMENT_TOP_LEFT, WHITE, temp_mem_arena)) {
                return false;
            }
        }
    }

    // Draw inventory slots.
    const t_s32 row_cnt = world->player_inv_open ? PLAYER_INVENTORY_ROW_CNT : 1;

    for (t_s32 r = 0; r < row_cnt; r++) {
        for (t_s32 c = 0; c < PLAYER_INVENTORY_COLUMN_CNT; c++) {
            const s_inventory_slot* const slot = &world->player_inv_slots[r][c];
            const s_v2 slot_pos = PlayerInventorySlotPos(r, c, ui_size);
            const u_v4 slot_color = r == 0 && c == world->player_inv_hotbar_slot_selected ? YELLOW : WHITE;

            if (!RenderInventorySlot(rendering_context, *slot, slot_pos, slot_color, textures, fonts, temp_mem_arena)) {
                return false;
            }
        }
    }

    return true;
}

bool RenderWorldUI(const s_world* const world, const s_game_render_context* const context, const s_texture_group* const textures, const s_font_group* const fonts) {
    const s_v2_s32 ui_size = UISize(context->rendering_context.window_size);
    const s_v2 mouse_ui_pos = DisplayToUIPos(context->mouse_pos, context->rendering_context.window_size);

    RenderTileHighlight(world, &context->rendering_context, context->mouse_pos);

    //
    // Popup Texts
    //
    for (t_s32 i = 0; i < POPUP_TEXT_LIMIT; i++) {
        const s_popup_text* const popup = &world->popup_texts[i];

        if (popup->alpha <= POPUP_TEXT_INACTIVITY_ALPHA_THRESH) {
            continue;
        }

        const s_v2 popup_display_pos = CameraToDisplayPos(popup->pos, &world->cam, context->rendering_context.window_size);
        const s_v2 popup_ui_pos = DisplayToUIPos(popup_display_pos, context->rendering_context.window_size);
        const u_v4 popup_blend = {1.0f, 1.0f, 1.0f, popup->alpha};

        assert(popup->str[0] != '\0' && "Popup text string cannot be empty!\n");

        if (!RenderStr(&context->rendering_context, (s_char_array_view)ARRAY_FROM_STATIC(popup->str), fonts, ek_font_eb_garamond_32, popup_ui_pos, ALIGNMENT_CENTER, popup_blend, context->temp_mem_arena)) {
            return false;
        }
    }

    //
    // Death Text
    //
    if (world->player.killed) {
        if (!RenderStr(&context->rendering_context, (s_char_array_view)ARRAY_FROM_STATIC(DEATH_TEXT), fonts, ek_font_eb_garamond_48, (s_v2){ui_size.x / 2.0f, ui_size.y / 2.0f}, ALIGNMENT_CENTER, WHITE, context->temp_mem_arena)) {
            return false;
        }
    }

    //
    // Player Health
    //
    {
        const s_v2 hp_pos = {
            ui_size.x * PLAYER_HP_POS_PERC.x,
            ui_size.y * PLAYER_HP_POS_PERC.y
        };

        const s_rect hp_bar_rect = {
            hp_pos.x - PLAYER_HP_BAR_WIDTH,
            hp_pos.y,
            PLAYER_HP_BAR_WIDTH,
            PLAYER_HP_BAR_HEIGHT
        };

        RenderBarHor(&context->rendering_context, hp_bar_rect, (t_r32)world->player.hp / world->core.player_hp_max, WHITE, BLACK);

        const s_v2 hp_str_pos = {
            hp_pos.x - hp_bar_rect.width - 10.0f,
            hp_pos.y + (hp_bar_rect.height / 2.0f)
        };

        char hp_str[8] = {0};
        snprintf(hp_str, sizeof(hp_str), "%d/%d", world->player.hp, world->core.player_hp_max);

        if (!RenderStr(&context->rendering_context, (s_char_array_view)ARRAY_FROM_STATIC(hp_str), fonts, ek_font_eb_garamond_28, hp_str_pos, ALIGNMENT_CENTER_RIGHT, WHITE, context->temp_mem_arena)) {
            return false;
        }
    }

    if (!RenderPlayerInventory(&context->rendering_context, world, textures, fonts, context->temp_mem_arena)) {
        return false;
    }

    //
    // Mouse Hover String
    //
    if (world->mouse_hover_str[0]) {
        if (!RenderStr(&context->rendering_context, (s_char_array_view)ARRAY_FROM_STATIC(world->mouse_hover_str), fonts, ek_font_eb_garamond_24, mouse_ui_pos, ALIGNMENT_TOP_LEFT, WHITE, context->temp_mem_arena)) {
            return false;
        }
    }

    //
    // Mouse Item Quantity
    //
    if (world->mouse_item_held_quantity > 0) {
        RenderSprite(&context->rendering_context, g_item_type_infos[world->mouse_item_held_type].icon_spr, textures, mouse_ui_pos, (s_v2){0.5f, 0.5f}, (s_v2){1.0f, 1.0f}, 0.0f, WHITE);
    }

    return true;
}
