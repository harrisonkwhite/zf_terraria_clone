#pragma once

#include "world_public.h"
#include "assets.h"
#include "items.h"
#include "tiles.h"

// ============================================================
// @section: External Forward Declarations

struct t_inventory;
struct t_tilemap;
struct t_camera;

// ==================================================

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

void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets);

// ==================================================

constexpr zcl::t_i32 k_pop_up_death_time_limit = 15;
constexpr zcl::t_f32 k_pop_up_lerp_factor = 0.15f;

struct t_player_entity {
    zcl::t_v2 pos;
    zcl::t_v2 vel;
    zcl::t_b8 jumping;

    zcl::t_i32 item_use_time;
};

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

    zcl::t_i32 player_health;
    zcl::t_i32 player_health_limit;

    t_inventory *player_inventory;

    t_tilemap *tilemap;

    t_player_entity player_entity;

    t_camera *camera;

    t_pop_ups pop_ups;

    struct {
        zcl::t_i32 player_inventory_open;
        zcl::t_i32 player_inventory_hotbar_slot_selected_index;

        t_item_type_id cursor_held_item_type_id;
        zcl::t_i32 cursor_held_quantity;
    } ui;
};

void ProcessItemUsage(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);
