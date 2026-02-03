#pragma once

#include "world_public.h"
#include "assets.h"
#include "tiles.h"

// ============================================================
// @section: External Forward Declarations

struct t_inventory;
struct t_camera;

// ==================================================

namespace world {
    // @note: If inventories generically ever become 2D, this can be dropped and you can just get this from the actual inventory handle.
    constexpr zcl::t_i32 k_player_inventory_width = 7;
    constexpr zcl::t_i32 k_player_inventory_height = 4;

    constexpr zcl::t_f32 k_gravity = 0.2f;

    // ============================================================
    // @section: Tilemap

    constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

    struct t_tilemap;

    t_tilemap *CreateTilemap(zcl::t_arena *const arena);

    zcl::t_b8 CheckTilemapPosInBounds(const zcl::t_v2_i pos);

    zcl::t_v2_i ConvertScreenToTilemapPos(const zcl::t_v2 pos_screen, const zcl::t_v2_i screen_size, const t_camera *const camera);

    // The tile position MUST be empty.
    void AddTile(t_tilemap *const tm, const zcl::t_v2_i pos, const t_tile_type_id tile_type);

    // The tile position MUST NOT be empty.
    void RemoveTile(t_tilemap *const tm, const zcl::t_v2_i pos);

    void HurtTile(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage);

    // Is the tile at the given position empty?
    zcl::t_b8 CheckTile(const t_tilemap *const tm, const zcl::t_v2_i pos);

    zcl::t_rect_i CalcTilemapRectSpan(const zcl::t_rect_f rect);

    zcl::t_b8 CheckTileCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

    void ProcessTileCollisions(const t_tilemap *const tilemap, zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin);

    void RenderTilemap(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets);

    // ==================================================

    // ============================================================
    // @section: Player

    // Data about the player which transcends deaths, so things like the inventory for example.
    struct t_player_meta;

    // Data associated with the actual player instance in the world, and which exists for the lifetime of that instance.
    struct t_player_entity;

    t_player_meta *PlayerCreateMeta(zcl::t_arena *const arena);

    t_player_entity *PlayerCreateEntity(const t_player_meta *const player_meta, const zcl::t_v2 pos, zcl::t_arena *const arena);

    t_inventory *PlayerGetInventory(t_player_meta *const player_meta);

    zcl::t_v2 PlayerGetPos(t_player_entity *const player_entity);

    zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 pos);

    void PlayerProcessMovement(t_player_entity *const player_entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state);

    void PlayerProcessInventoryHotbarUpdate(t_player_meta *const player_meta, const zgl::t_input_state *const input_state);

    void PlayerProcessItemUsage(const t_player_meta *const player_meta, t_player_entity *const player_entity, const t_tilemap *const tilemap, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);

    void PlayerRender(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets);

    // ==================================================

    // ============================================================
    // @section: UI

    struct t_ui;

    t_ui *UICreate(zcl::t_arena *const arena);

    void UIPlayerInventoryProcessInteraction(t_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state);

    void UIRenderTileHighlight(const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_camera *const camera);

    struct t_pop_ups;

    void UIRenderPopUps(const zgl::t_rendering_context rc, const t_pop_ups *const pop_ups, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void UIRenderPlayerInventory(const t_ui *const ui, const zgl::t_rendering_context rc, const t_inventory *const player_inventory, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void UIRenderPlayerHealth(const zgl::t_rendering_context rc);

    void UIRenderCursorHeldItem(const t_ui *const ui, const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_assets *const assets, zcl::t_arena *const temp_arena);

    // ==================================================
}
