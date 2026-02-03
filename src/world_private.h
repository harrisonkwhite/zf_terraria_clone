#pragma once

#include "world_public.h"
#include "assets.h"
#include "items.h"
#include "tiles.h"

// @todo: Organise header better.

// ============================================================
// @section: External Forward Declarations

struct t_inventory;
struct t_camera;

// ==================================================

namespace world {
    // ============================================================
    // @section: Tilemap

    constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

    struct t_tilemap;

    t_tilemap *TilemapCreate(zcl::t_arena *const arena);

    zcl::t_b8 TilemapPosCheckInBounds(const zcl::t_v2_i pos);

    // The tile position MUST be empty.
    void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i pos, const t_tile_type_id tile_type);

    // The tile position MUST NOT be empty.
    void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i pos);

    void TilemapHurt(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage);

    // Is the tile at the given position empty?
    zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i pos);

    zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect);

    zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

    void TilemapProcessCollisions(const t_tilemap *const tilemap, zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin);

    void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets);

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

    void UITick(t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);

    // ==================================================

    // @note: If inventories generically ever become 2D, this can be dropped and you can just get this from the actual inventory handle.
    constexpr zcl::t_i32 k_player_inventory_width = 7;
    constexpr zcl::t_i32 k_player_inventory_height = 4;

    constexpr zcl::t_f32 k_gravity = 0.2f;

    constexpr zcl::t_i32 k_pop_up_death_time_limit = 15;
    constexpr zcl::t_f32 k_pop_up_lerp_factor = 0.15f;

    struct t_pop_up {
        zcl::t_v2 pos;
        zcl::t_v2 vel;

        zcl::t_i32 death_time;

        zcl::t_static_array<zcl::t_u8, 32> str_bytes;
        zcl::t_i32 str_byte_cnt;

        t_font_id font_id;
    };

    constexpr zcl::t_i32 k_pop_up_limit = 1024;

    struct t_pop_ups {
        zcl::t_static_array<t_pop_up, k_pop_up_limit> buf;
        zcl::t_static_bitset<k_pop_up_limit> activity;
    };

    struct t_world {
        zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?

        t_tilemap *tilemap;

        t_player_entity *player_entity;
        t_player_meta *player_meta;

        t_camera *camera;

        t_pop_ups pop_ups;

        struct {
            zcl::t_i32 player_inventory_open;
            zcl::t_i32 player_inventory_hotbar_slot_selected_index;

            t_item_type_id cursor_held_item_type_id;
            zcl::t_i32 cursor_held_quantity;
        } ui;
    };
}
