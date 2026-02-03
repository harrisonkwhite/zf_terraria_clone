#pragma once

#include "world_public.h"
#include "assets.h"
#include "items.h"
#include "npcs.h"
#include "tiles.h"

// ============================================================
// @section: External Forward Declarations

struct t_camera;

// ==================================================

namespace world {
    constexpr zcl::t_f32 k_gravity = 0.2f;

    // ============================================================
    // @section: Inventory

    struct t_inventory;

    struct t_inventory_slot {
        t_item_type_id item_type_id;
        zcl::t_i32 quantity;
    };

    t_inventory *InventoryCreate(const zcl::t_v2_i size, zcl::t_arena *const arena);

    // Returns the quantity that couldn't be added due to the inventory getting filled (0 for all added).
    zcl::t_i32 InventoryAdd(t_inventory *const inventory, const t_item_type_id item_type_id, zcl::t_i32 quantity);

    // Returns the quantity that couldn't be added due to the slot getting filled (0 for all added).
    zcl::t_i32 InventoryAddAt(t_inventory *const inventory, const zcl::t_v2_i slot_pos, const t_item_type_id item_type_id, const zcl::t_i32 quantity);

    // Returns the quantity that couldn't be removed due to there not being enough of the item in the slot.
    zcl::t_i32 InventoryRemoveAt(t_inventory *const inventory, const zcl::t_v2_i slot_pos, const zcl::t_i32 quantity);

    t_inventory_slot InventoryGet(const t_inventory *const inventory, const zcl::t_v2_i slot_pos);

    zcl::t_v2_i InventoryGetSize(const t_inventory *const inventory);

    // ==================================================

    // ============================================================
    // @section: Tilemap

    constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

    struct t_tilemap;

    t_tilemap *TilemapCreate(zcl::t_arena *const arena);

    zcl::t_b8 TilemapCheckTilePosInBounds(const zcl::t_v2_i pos);

    // The tile position MUST be empty.
    void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type);

    // The tile position MUST NOT be empty.
    void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos);

    void TilemapHurt(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage);

    // Is the tile at the given position empty?
    zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i tile_pos);

    zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect);

    zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

    zcl::t_v2 TilemapMoveContact(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

    void TilemapProcessCollisions(const t_tilemap *const tilemap, zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin);

    void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets);

    zcl::t_v2_i ScreenToTilePos(const zcl::t_v2 pos_screen, const zcl::t_v2_i screen_size, const t_camera *const camera);

    // ==================================================

    // ============================================================
    // @section: Player

    // Data about the player which transcends deaths, so things like the inventory for example.
    struct t_player_meta;

    // Data associated with the actual player instance in the world, and which exists for the lifetime of that instance.
    struct t_player_entity;

    t_player_meta *PlayerCreateMeta(zcl::t_arena *const arena);

    t_player_entity *PlayerCreateEntity(const t_player_meta *const player_meta, const t_tilemap *const tilemap, zcl::t_arena *const arena);

    t_inventory *PlayerGetInventory(t_player_meta *const player_meta);

    zcl::t_v2 PlayerGetPos(t_player_entity *const player_entity);

    zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 pos);

    void PlayerProcessMovement(t_player_entity *const player_entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state);

    void PlayerProcessInventoryHotbarUpdates(t_player_meta *const player_meta, const zgl::t_input_state *const input_state);

    void PlayerProcessItemUsage(const t_player_meta *const player_meta, t_player_entity *const player_entity, const t_tilemap *const tilemap, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);

    void PlayerRender(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets);

    // ==================================================

    // ============================================================
    // @section: NPCs

    struct t_npc_manager;

    struct t_npc_id {
        zcl::t_i32 index;
        zcl::t_i32 version;
    };

    t_npc_manager *NPCManagerCreate(zcl::t_arena *const arena);

    t_npc_id NPCSpawn(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id);

    zcl::t_b8 NPCCheckExists(const t_npc_manager *const manager, const t_npc_id id);

    void NPCsUpdate(t_npc_manager *const manager, const t_tilemap *const tilemap);

    void NPCsRender(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets);

    // ==================================================

    // ============================================================
    // @section: UI

    struct t_ui;

    t_ui *UICreate(zcl::t_arena *const arena);

    void UIProcessPlayerInventoryInteraction(t_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state);

    void UIRenderTileHighlight(const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_camera *const camera);

    struct t_pop_ups;

    void UIRenderPopUps(const zgl::t_rendering_context rc, const t_pop_ups *const pop_ups, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void UIRenderPlayerInventory(const t_ui *const ui, const zgl::t_rendering_context rc, const t_inventory *const player_inventory, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void UIRenderPlayerHealth(const zgl::t_rendering_context rc);

    void UIRenderCursorHeldItem(const t_ui *const ui, const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_assets *const assets, zcl::t_arena *const temp_arena);

    // ==================================================
}
