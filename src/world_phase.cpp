#include "world_phase.h"

#include "camera.h"
#include "tiles.h"
#include "player.h"
#include "npcs.h"
#include "item_drops.h"
#include "world_gen.h"
#include "pop_ups.h"
#include "inventories.h"
#include "stray.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_gravity = 0.2f;

// @temp: Would rather not have multiple sources of truth.
constexpr zcl::t_v2_i k_tilemap_size = {8000, 400};
constexpr zcl::t_v2_i k_tilemap_chunk_size = {400, 20};

constexpr zcl::t_i32 k_player_respawn_break_duration = 120;

constexpr zcl::t_f32 k_ui_tile_highlight_alpha = 0.6f;
constexpr zcl::t_v2 k_ui_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_ui_player_health_bar_size = {240.0f, 24.0f};
constexpr zcl::t_f32 k_ui_player_health_bar_bg_alpha = 0.4f;
constexpr zcl::t_v2 k_ui_player_inventory_offs_top_left = {48.0f, 48.0f};
constexpr zcl::t_f32 k_ui_player_inventory_slot_size = 48.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_distance = 64.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_bg_alpha = 0.4f;

struct t_world_phase {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead? Maybe as a seed from the title screen?

    // t_tilemap *tilemap;
    t_tilemap *tilemap;

    t_player_entity *player_entity;
    t_player_meta *player_meta;
    zcl::t_i32 player_respawn_break;

    t_npc_manager npc_manager;

    t_item_drop_manager *item_drop_manager;

    t_pop_up_manager pop_up_manager;

    t_camera *camera;

    struct {
        zcl::t_i32 player_inventory_open;

        t_item_type_id cursor_held_item_type_id;
        zcl::t_i32 cursor_held_quantity;
    } ui;
};

t_world_phase *WorldPhaseInit(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    const auto result = zcl::ArenaPush<t_world_phase>(arena);

    result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    const auto tilemap_core = WorldGen(k_tilemap_size, result->rng, arena, temp_arena);
    result->tilemap = TilemapCreate(tilemap_core, k_tilemap_chunk_size, arena);

    result->player_meta = PlayerMetaCreate(arena);

    result->player_entity = PlayerEntityCreate(result->player_meta, result->tilemap, arena);

    result->item_drop_manager = ItemDropManagerCreate(arena);

    result->camera = CameraCreate(PlayerGetPosition(result->player_entity), 2.0f, 0.3f, arena);

    NPCSpawn(&result->npc_manager, {k_tile_size * k_tilemap_size.x * 0.5f, 0.0f}, ek_npc_type_id_slime);

    return result;
}

// Returns true iff a slot is hovered.
[[nodiscard]] static zcl::t_b8 CalcPlayerInventoryUIHoveredSlotPos(const t_inventory *const inventory, const zcl::t_b8 inventory_open, const zcl::t_v2 cursor_position, zcl::t_v2_i *const o_slot_pos) {
    const zcl::t_v2 cursor_pos_rel_to_inventory_top_left = cursor_position - k_ui_player_inventory_offs_top_left;

    const zcl::t_v2_i inventory_size = InventoryGetSize(inventory);
    const zcl::t_v2 inventory_size_in_pixels = k_ui_player_inventory_slot_distance * zcl::V2IToF(inventory_size);

    if (cursor_pos_rel_to_inventory_top_left.x >= 0.0f && cursor_pos_rel_to_inventory_top_left.y >= 0.0f && cursor_pos_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_pos_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_pos = {
            static_cast<zcl::t_i32>(zcl::Floor(cursor_pos_rel_to_inventory_top_left.x / k_ui_player_inventory_slot_distance)),
            static_cast<zcl::t_i32>(zcl::Floor(cursor_pos_rel_to_inventory_top_left.y / k_ui_player_inventory_slot_distance)),
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

static zcl::t_rect_f CalcPlayerInventoryUISlotRect(const zcl::t_v2_i slot_pos) {
    const zcl::t_v2 slot_pos_screen = k_ui_player_inventory_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_ui_player_inventory_slot_distance);
    const zcl::t_v2 slot_size = {k_ui_player_inventory_slot_size, k_ui_player_inventory_slot_size};

    return zcl::RectCreateF(slot_pos_screen, slot_size);
}

static void ProcessPlayerInventoryUIInteraction(t_world_phase *const world_phase, const zgl::t_input_state *const input_state) {
    const auto player_inventory = PlayerGetInventory(world_phase->player_meta);
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        world_phase->ui.player_inventory_open = !world_phase->ui.player_inventory_open;
    }

    if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
        zcl::t_v2_i slot_hovered_pos;

        if (CalcPlayerInventoryUIHoveredSlotPos(player_inventory, world_phase->ui.player_inventory_open, cursor_pos, &slot_hovered_pos)) {
            const auto slot = InventoryGet(player_inventory, slot_hovered_pos);

            if (world_phase->ui.cursor_held_quantity == 0) {
                world_phase->ui.cursor_held_item_type_id = slot.item_type_id;
                world_phase->ui.cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(player_inventory, slot_hovered_pos, slot.quantity);
            } else {
                if (slot.quantity == 0) {
                    InventoryAddAt(player_inventory, slot_hovered_pos, world_phase->ui.cursor_held_item_type_id, world_phase->ui.cursor_held_quantity);
                    world_phase->ui.cursor_held_quantity = 0;
                }
            }
        }
    }
}

t_world_phase_tick_result_id WorldPhaseTick(t_world_phase *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    t_world_phase_tick_result_id result_id = ek_world_phase_tick_result_id_normal;

    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    // ----------------------------------------
    // Player Respawn

    if (world->player_respawn_break > 0) {
        world->player_respawn_break--;
    } else {
        if (!PlayerCheckAlive(world->player_entity)) {
            PlayerEntityReset(world->player_entity, world->player_meta, world->tilemap);
            CameraSetPosition(world->camera, PlayerGetPosition(world->player_entity));
        }
    }

    // ------------------------------

    ProcessPlayerInventoryUIInteraction(world, input_state);

    if (PlayerCheckAlive(world->player_entity)) {
        PlayerUpdateTimers(world->player_entity);

        PlayerProcessInventoryHotbarUpdates(world->player_meta, input_state);

        PlayerUpdateMovement(world->player_entity, input_state, k_gravity, world->tilemap);

        PlayerProcessItemUsage(world->player_entity, input_state, world->player_meta, world->item_drop_manager, world->camera, world->tilemap, screen_size, temp_arena);
    }

    NPCsProcessAIs(&world->npc_manager, k_gravity, world->tilemap);

    ItemDropsProcessMovementAndCollection(world->item_drop_manager, world->player_meta, world->player_entity, k_gravity, world->tilemap, &world->pop_up_manager, world->rng);

    if (PlayerCheckAlive(world->player_entity)) {
        ProcessPlayerAndNPCCollisions(world->player_entity, &world->npc_manager, &world->pop_up_manager, world->rng, temp_arena);

        PlayerProcessDeath(world->player_entity);

        if (!PlayerCheckAlive(world->player_entity)) {
            world->player_respawn_break = k_player_respawn_break_duration;
        }
    }

    NPCsProcessDeaths(&world->npc_manager);

    // @todo: Camera shake helpers.
    // @todo: Target position needs to be cached inside camera struct and updated via a distinct function.
    CameraMove(world->camera, PlayerGetPosition(world->player_entity));

    PopUpsUpdate(&world->pop_up_manager);

    return result_id;
}

void WorldPhaseRender(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, rc.screen_size);
    zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix, true, k_bg_color);

    TilemapRender(world->tilemap, rc, CalcCameraTilemapRect(world->camera, world->tilemap, rc.screen_size), assets);

    if (PlayerCheckAlive(world->player_entity)) {
        PlayerRender(world->player_entity, rc, assets);
    }

    NPCsRender(&world->npc_manager, rc, assets);

    ItemDropsRender(world->item_drop_manager, rc, assets);

    zgl::RendererPassEnd(rc);
}

static zcl::t_str_mut DetermineCursorHoverStr(const zcl::t_v2 cursor_pos, const t_inventory *const player_inventory, const zcl::t_b8 player_inventory_open, const t_npc_manager *const npc_manager, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
    constexpr zcl::t_i32 k_str_len_limit = 32;

    {
        zcl::t_v2_i player_inventory_slot_hovered_pos;

        if (CalcPlayerInventoryUIHoveredSlotPos(player_inventory, player_inventory_open, cursor_pos, &player_inventory_slot_hovered_pos)) {
            const t_inventory_slot player_inventory_slot_hovered = InventoryGet(player_inventory, player_inventory_slot_hovered_pos);

            if (player_inventory_slot_hovered.quantity > 0) {
                return zcl::StrClone(g_item_types[player_inventory_slot_hovered.item_type_id].name, arena);
            }
        }
    }

    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(npc_manager->activity, i)) {
            continue;
        }

        const auto npc = &npc_manager->buf[i];
        const zcl::t_rect_f npc_collider = NPCGetCollider(npc->pos, npc->type_id);
        const zcl::t_rect_f npc_collider_screen = zcl::RectCreateF(CameraToScreenPos(zcl::RectGetPos(npc_collider), camera, screen_size), zcl::RectGetSize(npc_collider) * CameraGetScale(camera));

        if (zcl::CheckPointInRect(cursor_pos, npc_collider_screen)) {
            return zcl::StrClone(g_npc_types[npc->type_id].name, arena);
        }
    }

    return {};
}

static void RenderItemUI(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    SpriteRender(g_item_types[item_type_id].icon_sprite_id, rc, assets, pos, zcl::k_origin_center, 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rc, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *FontGet(assets, ek_font_id_eb_garamond_24), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

void WorldPhaseRenderUI(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    PopUpsRender(&world->pop_up_manager, rc, world->camera, assets, temp_arena);

    // ----------------------------------------
    // Player Health

    {
        const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(rc.screen_size.x) - k_ui_player_health_bar_offs_top_right.x - k_ui_player_health_bar_size.x, k_ui_player_health_bar_offs_top_right.y};
        const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_ui_player_health_bar_size);

        zgl::RendererSubmitRect(rc, health_bar_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_health_bar_bg_alpha));

        zgl::RendererSubmitRectOutlineOpaque(rc, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);

        zgl::RendererSubmitRect(rc, zcl::RectCreateF(health_bar_rect.x, health_bar_rect.y, health_bar_rect.width * (static_cast<zcl::t_f32>(PlayerGetHealth(world->player_entity)) / PlayerGetHealthLimit(world->player_meta)), health_bar_rect.height), zcl::k_color_white);
    }

    // ------------------------------

    // ----------------------------------------
    // Player Inventory

    {
        const auto inventory = PlayerGetInventory(world->player_meta);
        const zcl::t_v2_i inventory_size = InventoryGetSize(inventory);

        const zcl::t_i32 slot_cnt_y = world->ui.player_inventory_open ? inventory_size.y : 1;

        for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
            for (zcl::t_i32 slot_x = 0; slot_x < inventory_size.x; slot_x++) {
                const zcl::t_i32 slot_index = (slot_y * inventory_size.x) + slot_x;

                const auto slot = InventoryGet(inventory, {slot_x, slot_y});

                const auto ui_slot_rect = CalcPlayerInventoryUISlotRect({slot_x, slot_y});

                const auto ui_slot_color = slot_y == 0 && PlayerGetInventoryHotbarSlotSelectedIndex(world->player_meta) == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
                ZCL_ASSERT(ui_slot_color.a == 1.0f);

                zgl::RendererSubmitRect(rc, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_inventory_slot_bg_alpha));
                zgl::RendererSubmitRectOutlineOpaque(rc, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

                if (slot.quantity > 0) {
                    RenderItemUI(slot.item_type_id, slot.quantity, rc, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
                }
            }
        }
    }

    // ------------------------------

    // ----------------------------------------
    // Player Death

    if (!PlayerCheckAlive(world->player_entity)) {
        const zcl::t_v2 screen_center = zcl::V2IToF(rc.screen_size) / 2.0f;
        zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("You were slain..."), *FontGet(assets, ek_font_id_eb_garamond_80), screen_center, zcl::k_color_white, temp_arena, zcl::k_origin_center);
    }

    // ------------------------------

    // ----------------------------------------
    // Cursor Held Item

    if (world->ui.cursor_held_quantity > 0) {
        RenderItemUI(world->ui.cursor_held_item_type_id, world->ui.cursor_held_quantity, rc, cursor_pos, assets, temp_arena);
    }

    // ------------------------------

    // ----------------------------------------
    // Cursor Hover String

    {
        const auto cursor_hover_str = DetermineCursorHoverStr(cursor_pos, PlayerGetInventory(world->player_meta), world->ui.player_inventory_open, &world->npc_manager, world->camera, rc.screen_size, temp_arena);

        if (!zcl::StrCheckEmpty(cursor_hover_str)) {
            zgl::RendererSubmitStr(rc, cursor_hover_str, *FontGet(assets, ek_font_id_eb_garamond_32), cursor_pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
        }
    }

    // ------------------------------
}

#if 0
// Returns true iff a slot is hovered.
[[nodiscard]] static zcl::t_b8 GetPlayerInventoryHoveredSlotPos(const t_inventory *const inventory, const zcl::t_b8 inventory_open, const zcl::t_v2 cursor_position, zcl::t_v2_i *const o_slot_pos) {
    const zcl::t_v2 cursor_pos_rel_to_inventory_top_left = cursor_position - k_ui_player_inventory_offs_top_left;

    const zcl::t_v2_i inventory_size = InventoryGetSize(inventory);
    const zcl::t_v2 inventory_size_in_pixels = k_ui_player_inventory_slot_distance * zcl::V2IToF(inventory_size);

    if (cursor_pos_rel_to_inventory_top_left.x >= 0.0f && cursor_pos_rel_to_inventory_top_left.y >= 0.0f && cursor_pos_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_pos_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_pos = {
            static_cast<zcl::t_i32>(zcl::Floor(cursor_pos_rel_to_inventory_top_left.x / k_ui_player_inventory_slot_distance)),
            static_cast<zcl::t_i32>(zcl::Floor(cursor_pos_rel_to_inventory_top_left.y / k_ui_player_inventory_slot_distance)),
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
#endif
