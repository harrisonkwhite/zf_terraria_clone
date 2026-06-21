#include "world_phase.h"

#include "camera.h"
#include "tiles.h"
#include "lighting.h"
#include "player.h"
#include "npcs.h"
#include "item_drops.h"
#include "world_gen.h"
#include "pop_ups.h"
#include "inventories.h"
#include "hitboxes.h"
#include "stray.h"

constexpr zcl::t_f32 k_gravity = 0.2f;

constexpr zcl::t_v2_i k_tilemap_size = {8000, 400};

constexpr zcl::t_i32 k_player_respawn_break_duration = 120;

constexpr zcl::t_i32 k_npc_spawn_interval = 300;
constexpr zcl::t_i32 k_npc_spawn_limit = 8; // @note: In the future this could vary based on biome and other factors.

constexpr zcl::t_f32 k_ui_tile_highlight_alpha = 0.6f;
constexpr zcl::t_v2 k_ui_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_ui_player_health_bar_size = {240.0f, 24.0f};
constexpr zcl::t_f32 k_ui_player_health_bar_bg_alpha = 0.4f;
constexpr zcl::t_v2 k_ui_player_inventory_offs_top_left = {48.0f, 48.0f};
constexpr zcl::t_f32 k_ui_player_inventory_slot_size = 48.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_distance = 64.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_bg_alpha = 0.4f;

struct t_world_phase {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead? Maybe as a seed from the title screen? Should this runtime RNG be distinct from that of the world generation?

    t_tilemap *tilemap;

    t_player_entity *player_entity;
    t_player_meta *player_meta;
    zcl::t_i32 player_respawn_break;

    t_npc_manager *npc_manager;
    zcl::t_i32 npc_spawn_time;

    t_item_drop_manager *item_drop_manager;

    t_hitbox_manager *hitbox_manager;

    t_pop_up_manager *pop_up_manager;

    t_camera *camera;

    struct {
        zcl::t_i32 player_inventory_open;

        t_item_type_id cursor_held_item_type_id;
        zcl::t_i32 cursor_held_quantity;
    } ui;

#ifdef ZCL_DEBUG
    struct {
        zcl::t_b8 render_hitboxes;
    } debug;
#endif
};

t_world_phase *WorldPhaseInit(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    const auto result = zcl::ArenaPush<t_world_phase>(arena);

    result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    const auto tilemap_core = WorldGen(k_tilemap_size, result->rng, arena, temp_arena);
    result->tilemap = TilemapCreate(tilemap_core, arena);

    result->player_meta = PlayerMetaCreate(arena);

    result->player_entity = PlayerEntityCreate(result->player_meta, result->tilemap, arena);

    result->npc_manager = NPCManagerCreate(arena);

    result->item_drop_manager = ItemDropManagerCreate(arena);

    result->hitbox_manager = HitboxManagerCreate(128, arena);

    result->pop_up_manager = PopUpManagerCreate(arena);

    result->camera = CameraCreate(PlayerGetPosition(result->player_entity), 2.0f, 0.3f, arena);

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

static zcl::t_rect_f CalcPlayerInventoryUISlotRect(const zcl::t_v2_i slot_pos, const zcl::t_b8 selected) {
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
            if (world_phase->ui.cursor_held_quantity == 0) {
                const auto slot = InventoryGet(player_inventory, slot_hovered_pos);

                world_phase->ui.cursor_held_item_type_id = slot.item_type_id;
                world_phase->ui.cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(player_inventory, slot_hovered_pos, slot.quantity);
            } else {
                const zcl::t_i32 added_cnt = world_phase->ui.cursor_held_quantity - InventoryAddAt(player_inventory, slot_hovered_pos, world_phase->ui.cursor_held_item_type_id, world_phase->ui.cursor_held_quantity);

                if (added_cnt == 0) {
                    // Nothing could get added to the slot, so we'll have to do a swap instead.
                    const auto cursor_held_item_type_id_old = world_phase->ui.cursor_held_item_type_id;
                    const auto cursor_held_item_type_quantity_old = world_phase->ui.cursor_held_quantity;

                    const auto slot = InventoryGet(player_inventory, slot_hovered_pos);

                    world_phase->ui.cursor_held_item_type_id = slot.item_type_id;
                    world_phase->ui.cursor_held_quantity = slot.quantity;

                    InventoryRemoveAt(player_inventory, slot_hovered_pos, slot.quantity);
                    InventoryAddAt(player_inventory, slot_hovered_pos, cursor_held_item_type_id_old, cursor_held_item_type_quantity_old);
                } else {
                    world_phase->ui.cursor_held_quantity -= added_cnt;
                }
            }
        }
    }
}

t_world_phase_tick_result_id WorldPhaseTick(t_world_phase *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    t_world_phase_tick_result_id result_id = ek_world_phase_tick_result_id_normal;

    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    HitboxesClear(world->hitbox_manager);

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

    // ----------------------------------------
    // NPC Spawning

    if (NPCsGetCount(world->npc_manager) < k_npc_spawn_limit) {
        if (world->npc_spawn_time < k_npc_spawn_interval) {
            world->npc_spawn_time++;
        } else {
            const t_npc_type_id npc_type_id = ek_npc_type_id_slime; // @temp: Vary later.

            const auto npc_spawn_pos_calc = [world, screen_size](zcl::t_v2 *const o_pos) {
                const auto npc_collider_size = NPCGetColliderSize(npc_type_id);

                const auto camera_rect = CameraCalcRect(world->camera, screen_size);
                constexpr zcl::t_f32 k_camera_rect_offs = 128.0f;

                constexpr zcl::t_i32 k_trial_limit = 1000;
                zcl::t_i32 trial_cnt = 0;

                const zcl::t_rect_f spawn_rect = {
                    camera_rect.x - npc_collider_size.x - k_camera_rect_offs,
                    camera_rect.y - npc_collider_size.y - k_camera_rect_offs,
                    camera_rect.width + ((npc_collider_size.x + k_camera_rect_offs) * 2),
                    camera_rect.height + ((npc_collider_size.y + k_camera_rect_offs) * 2),
                };

                zcl::t_rect_f collider;

                do {
                    if (trial_cnt >= k_trial_limit) {
                        return false;
                    }

                    *o_pos = {
                        zcl::RectGetLeft(spawn_rect) + (zcl::RandGenPerc(world->rng) * spawn_rect.width),
                        zcl::RectGetTop(spawn_rect) + (zcl::RandGenPerc(world->rng) * spawn_rect.height),
                    };

                    *o_pos = MakeContactWithTilemap(*o_pos, zcl::ek_cardinal_direction_down, zcl::RectGetSize(NPCGetCollider(*o_pos, npc_type_id)), k_npc_origin, world->tilemap);
                    collider = NPCGetCollider(*o_pos, npc_type_id);

                    trial_cnt++;
                } while (TilemapCheckCollision(world->tilemap, collider) || zcl::CheckInters(camera_rect, collider));

                return true;
            };

            zcl::t_v2 npc_spawn_pos;

            if (npc_spawn_pos_calc(&npc_spawn_pos)) {
                NPCSpawn(world->npc_manager, npc_spawn_pos, npc_type_id, world->rng);
            }

            world->npc_spawn_time = 0;
        }
    } else {
        world->npc_spawn_time = 0;
    }

    // ------------------------------

    if (PlayerCheckAlive(world->player_entity)) {
        ProcessPlayerInventoryUIInteraction(world, input_state);

        PlayerUpdateTimers(world->player_entity);

        PlayerProcessInventoryHotbarUpdates(world->player_meta, input_state);

        PlayerUpdateMovement(world->player_entity, input_state, k_gravity, world->tilemap);

        PlayerProcessItemUsage(world->player_entity, input_state, world->player_meta, world->npc_manager, world->item_drop_manager, world->camera, world->tilemap, world->hitbox_manager, screen_size, temp_arena);
    } else {
        world->ui.player_inventory_open = false;
    }

    NPCsProcessAIs(world->npc_manager, k_gravity, world->player_entity, world->tilemap, world->rng);

    NPCsSubmitHitboxes(world->npc_manager, world->hitbox_manager);

    ItemDropsProcessMovementAndCollection(world->item_drop_manager, world->player_meta, world->player_entity, k_gravity, world->tilemap, world->pop_up_manager, world->rng, temp_arena);

    if (PlayerCheckAlive(world->player_entity)) {
        PlayerProcessHitboxCollisions(world->player_entity, HitboxesLoadAll(world->hitbox_manager), world->pop_up_manager, world->rng);

        PlayerProcessDeath(world->player_entity);

        if (!PlayerCheckAlive(world->player_entity)) {
            world->player_respawn_break = k_player_respawn_break_duration;
        }
    }

    NPCsProcessHitboxCollisions(world->npc_manager, HitboxesLoadAll(world->hitbox_manager), world->pop_up_manager, world->rng);

    NPCsProcessDeaths(world->npc_manager);

    // @todo: Pulling position state from the player when player is inactive is a bit dodgy? Perhaps camera target position needs to be cached inside camera struct and updated via a distinct function.
    CameraMove(world->camera, PlayerGetPosition(world->player_entity));

    PopUpsUpdate(world->pop_up_manager);

#ifdef ZCL_DEBUG
    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_f1)) {
        world->debug.render_hitboxes = !world->debug.render_hitboxes;
    }
#endif

    return result_id;
}

void WorldPhaseRender(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, rc.screen_size);
    zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix, true, k_sky_color);

    const auto camera_tilemap_rect = CalcCameraTilemapRect(world->camera, world->tilemap, rc.screen_size);

    TilemapRender(world->tilemap, rc, camera_tilemap_rect, assets);

    if (PlayerCheckAlive(world->player_entity)) {
        PlayerRender(world->player_entity, rc, assets);
    }

    NPCsRender(world->npc_manager, rc, assets);

    ItemDropsRender(world->item_drop_manager, rc, assets);

#ifdef ZCL_DEBUG
    if (world->debug.render_hitboxes) {
        HitboxesRender(world->hitbox_manager, rc);
    }
#endif

    // ----------------------------------------
    // Lighting Setup

    {
        const auto lightmap = LightmapCreate(zcl::RectGetSize(camera_tilemap_rect), temp_arena);

        for (zcl::t_i32 y = 0; y < camera_tilemap_rect.height; y++) {
            for (zcl::t_i32 x = 0; x < camera_tilemap_rect.width; x++) {
                const zcl::t_i32 tx = camera_tilemap_rect.x + x;
                const zcl::t_i32 ty = camera_tilemap_rect.y + y;

                if (!TilemapCheck(world->tilemap, {tx, ty})) {
                    LightmapSetLevel(lightmap, {x, y}, k_light_level_limit);
                }
            }
        }

        LightmapPropagate(lightmap, temp_arena);

        LightmapRender(lightmap, rc, zcl::V2IToF(zcl::RectGetPos(camera_tilemap_rect) * k_tile_size), k_tile_size);
    }

    // ------------------------------

    zgl::RendererPassEnd(rc);
}

static zcl::t_str_mut DetermineCursorHoverStr(const zcl::t_v2 cursor_pos, const t_inventory *const player_inventory, const zcl::t_b8 player_inventory_open, const t_npc_manager *const npc_manager, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    constexpr zcl::t_i32 k_str_len_limit = 32;

    {
        zcl::t_v2_i player_inventory_slot_hovered_pos;

        if (CalcPlayerInventoryUIHoveredSlotPos(player_inventory, player_inventory_open, cursor_pos, &player_inventory_slot_hovered_pos)) {
            const t_inventory_slot player_inventory_slot_hovered = InventoryGet(player_inventory, player_inventory_slot_hovered_pos);

            if (player_inventory_slot_hovered.quantity > 0) {
                return InventoryDetermineItemStr(player_inventory_slot_hovered.item_type_id, player_inventory_slot_hovered.quantity, arena);
            }
        }
    }

    const auto npc_ids = NPCsLoad(npc_manager, temp_arena);

    for (zcl::t_i32 i = 0; i < npc_ids.len; i++) {
        const auto npc_id = npc_ids[i];

        const auto npc_type_id = NPCGetTypeID(npc_manager, npc_id);
        const zcl::t_rect_f npc_collider = NPCGetCollider(NPCGetPosition(npc_manager, npc_id), npc_type_id);
        const zcl::t_rect_f npc_collider_screen = zcl::RectCreateF(CameraToScreenPos(zcl::RectGetPos(npc_collider), camera, screen_size), zcl::RectGetSize(npc_collider) * CameraGetScale(camera));

        if (zcl::CheckPointInRect(cursor_pos, npc_collider_screen)) {
            return zcl::StrClone(g_npc_types[npc_type_id].name, arena);
        }
    }

    return {};
}

static void RenderItemUI(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    const auto item_type = &g_item_types[item_type_id];

    SpriteRender(item_type->sprite_id, rc, assets, pos, zcl::k_origin_center, (item_type->flags & ek_item_type_flag_sprite_diagonal) ? -zcl::k_pi / 4.0f : 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rc, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *FontGet(assets, ek_font_id_roboto_20), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

void WorldPhaseRenderUI(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    // ----------------------------------------
    // NPC Health

    {
        const auto npc_ids = NPCsLoad(world->npc_manager, temp_arena);

        for (zcl::t_i32 i = 0; i < npc_ids.len; i++) {
            const zcl::t_v2 npc_pos = NPCGetPosition(world->npc_manager, npc_ids[i]);
            const auto npc_type_id = NPCGetTypeID(world->npc_manager, npc_ids[i]);
            const auto npc_collider = NPCGetCollider(npc_pos, npc_type_id);

            const zcl::t_f32 health_bar_perc = static_cast<zcl::t_f32>(NPCGetHealth(world->npc_manager, npc_ids[i])) / g_npc_types[npc_type_id].health_limit;

            const zcl::t_rect_f health_bar_rect_entirety_camera = {
                zcl::RectGetLeft(npc_collider) - 1.0f,
                zcl::RectGetBottom(npc_collider) + 3.0f,
                npc_collider.width,
                2.0f,
            };

            const zcl::t_v2 health_bar_rect_entirety_screen_pos = CameraToScreenPos(zcl::RectGetPos(health_bar_rect_entirety_camera), world->camera, rc.screen_size);
            const zcl::t_v2 health_bar_rect_entirety_screen_size = zcl::RectGetSize(health_bar_rect_entirety_camera) * CameraGetScale(world->camera);
            const auto health_bar_rect_entirety_screen = zcl::RectCreateF(health_bar_rect_entirety_screen_pos, health_bar_rect_entirety_screen_size);

            zgl::RendererSubmitRect(rc, health_bar_rect_entirety_screen, zcl::k_color_black);

            const auto health_bar_rect_partial_screen = zcl::RectCreateF(health_bar_rect_entirety_screen_pos, {health_bar_rect_entirety_screen_size.x * health_bar_perc, health_bar_rect_entirety_screen_size.y});

            zgl::RendererSubmitRect(rc, health_bar_rect_partial_screen, zcl::k_color_white);
        }
    }

    // ------------------------------

    // ----------------------------------------
    // Tile Highlight

    if (PlayerCheckAlive(world->player_entity)) {
        const t_inventory_slot hotbar_slot_selected = PlayerGetInventoryHotbarSlotSelected(world->player_meta);

        if (hotbar_slot_selected.quantity > 0 && g_item_types[hotbar_slot_selected.item_type_id].flags & ek_item_type_flag_show_tile_highlight) {
            zcl::t_v2_i tile_hovered_pos;

            if (LoadHoveredTilePositionIfInReach(cursor_pos, rc.screen_size, world->camera, PlayerGetPosition(world->player_entity), &tile_hovered_pos)) {
                const zcl::t_v2 tile_hovered_pos_world = zcl::V2IToF(tile_hovered_pos) * k_tile_size;

                const zcl::t_rect_f highlight_rect = zcl::RectCreateF(CameraToScreenPos(tile_hovered_pos_world, world->camera, rc.screen_size), zcl::t_v2{k_tile_size, k_tile_size} * CameraGetScale(world->camera));

                zgl::RendererSubmitRect(rc, highlight_rect, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, k_ui_tile_highlight_alpha));
            }
        }
    }

    // ------------------------------

    PopUpsRender(world->pop_up_manager, rc, world->camera, assets, temp_arena);

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

        // Render the slots.
        const zcl::t_i32 slot_cnt_y = world->ui.player_inventory_open ? inventory_size.y : 1;

        for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
            for (zcl::t_i32 slot_x = 0; slot_x < inventory_size.x; slot_x++) {
                const zcl::t_i32 slot_index = (slot_y * inventory_size.x) + slot_x;

                const auto slot = InventoryGet(inventory, {slot_x, slot_y});

                const zcl::t_b8 slot_selected = slot_y == 0 && PlayerGetInventoryHotbarSlotSelectedIndex(world->player_meta) == slot_x;

                const auto ui_slot_rect = CalcPlayerInventoryUISlotRect({slot_x, slot_y}, slot_selected);

                const auto ui_slot_color = slot_selected ? zcl::k_color_yellow : zcl::k_color_white;
                ZCL_ASSERT(ui_slot_color.a == 1.0f);

                zgl::RendererSubmitRect(rc, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_inventory_slot_bg_alpha));
                zgl::RendererSubmitRectOutlineOpaque(rc, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

                if (slot.quantity > 0) {
                    RenderItemUI(slot.item_type_id, slot.quantity, rc, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
                }
            }
        }

        // Render the name and quantity of the selected item.
        const auto hotbar_slot_selected = InventoryGet(PlayerGetInventory(world->player_meta), {PlayerGetInventoryHotbarSlotSelectedIndex(world->player_meta), 0});

        if (hotbar_slot_selected.quantity > 0) {
            const zcl::t_f32 hotbar_width = (k_ui_player_inventory_slot_distance * (inventory_size.x - 1)) + k_ui_player_inventory_slot_size;
            const zcl::t_v2 item_str_pos = k_ui_player_inventory_offs_top_left + zcl::t_v2{hotbar_width / 2.0f, -8.0f};
            const auto item_str = InventoryDetermineItemStr(hotbar_slot_selected.item_type_id, hotbar_slot_selected.quantity, temp_arena);

            zgl::RendererSubmitStr(rc, item_str, *FontGet(assets, ek_font_id_roboto_24), item_str_pos, zcl::k_color_white, temp_arena, zcl::k_origin_bottom_center);
        }
    }

    // ------------------------------

    // ----------------------------------------
    // Player Death

    if (!PlayerCheckAlive(world->player_entity)) {
        const zcl::t_v2 screen_center = zcl::V2IToF(rc.screen_size) / 2.0f;
        zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("You were slain..."), *FontGet(assets, ek_font_id_roboto_64), screen_center, zcl::k_color_white, temp_arena, zcl::k_origin_center);
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
        const auto cursor_hover_str = DetermineCursorHoverStr(cursor_pos, PlayerGetInventory(world->player_meta), world->ui.player_inventory_open, world->npc_manager, world->camera, rc.screen_size, temp_arena, temp_arena);

        if (!zcl::StrCheckEmpty(cursor_hover_str)) {
            zgl::RendererSubmitStr(rc, cursor_hover_str, *FontGet(assets, ek_font_id_roboto_28), cursor_pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
        }
    }

    // ------------------------------
}
