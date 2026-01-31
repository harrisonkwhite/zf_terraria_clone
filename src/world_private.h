#pragma once

#include "world_public.h"
#include "sprites.h"


// ============================================================
// @section: Items
// ============================================================

enum t_item_type_id : zcl::t_i32 {
    ek_item_type_id_dirt_block,
    ek_item_type_id_stone_block,
    ek_item_type_id_grass_block,

    ekm_item_type_id_cnt
};

struct t_item_type {
    zcl::t_str_rdonly name;
    t_sprite_id icon_sprite_id;
};

inline const zcl::t_static_array<t_item_type, ekm_item_type_id_cnt> g_item_types = {{
    {
        .name = ZCL_STR_LITERAL("Dirt Block"),
        .icon_sprite_id = ek_sprite_id_dirt_block_item_icon,
    },
    {
        .name = ZCL_STR_LITERAL("Stone Block"),
        .icon_sprite_id = ek_sprite_id_stone_block_item_icon,
    },
    {
        .name = ZCL_STR_LITERAL("Grass Block"),
        .icon_sprite_id = ek_sprite_id_grass_block_item_icon,
    },
}};

// ============================================================


// ============================================================
// @section: Inventories
// ============================================================

struct t_inventory;

struct t_inventory_slot {
    t_item_type_id item_type_id;
    zcl::t_i32 quantity;
};

t_inventory *InventoryCreate(const zcl::t_i32 slot_cnt, zcl::t_arena *const arena);

// Returns the quantity that couldn't be added due to the inventory getting filled (0 for all added).
zcl::t_i32 InventoryAdd(t_inventory *const inventory, const t_item_type_id item_type_id, zcl::t_i32 quantity);

// Returns the quantity that couldn't be added due to the slot getting filled (0 for all added).
zcl::t_i32 InventoryAddAt(t_inventory *const inventory, const zcl::t_i32 slot_index, const t_item_type_id item_type_id, const zcl::t_i32 quantity);

// Returns the quantity that couldn't be removed due to there not being enough of the item in the slot.
zcl::t_i32 InventoryRemoveAt(t_inventory *const inventory, const zcl::t_i32 slot_index, const zcl::t_i32 quantity);

t_inventory_slot InventoryGet(const t_inventory *const inventory, const zcl::t_i32 slot_index);

// ============================================================


// ============================================================
// @section: Tiles
// ============================================================

struct t_tile_type {
    t_sprite_id sprite;
};

enum t_tile_type_id : zcl::t_i8 {
    ek_tile_type_id_dirt,
    ek_tile_type_id_stone,
    ek_tile_type_id_grass,

    ekm_tile_type_id_cnt
};

constexpr zcl::t_static_array<t_tile_type, ekm_tile_type_id_cnt> k_tile_types = {{
    {.sprite = ek_sprite_id_dirt_tile},
    {.sprite = ek_sprite_id_stone_tile},
    {.sprite = ek_sprite_id_grass_tile},
}};

constexpr zcl::t_i32 k_tile_size = 8;

constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

// @todo: Could possibly be made opaque?
struct t_tilemap {
    zcl::t_static_bitset<k_tilemap_size.x * k_tilemap_size.y> activity;
    zcl::t_static_array<zcl::t_static_array<t_tile_type_id, k_tilemap_size.x>, k_tilemap_size.y> types;
};

zcl::t_b8 TilePosCheckInBounds(const zcl::t_v2_i pos);

void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type);

void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos);

zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i tile_pos);

zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect);

zcl::t_b8 TileCollisionCheck(const t_tilemap *const tilemap, const zcl::t_rect_f collider);

zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void ProcessTileCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap);

void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rendering_context, const t_assets *const assets);

// ============================================================


void WorldGen(zcl::t_rng *const rng, t_tilemap *const o_tilemap);

constexpr zcl::t_i32 k_player_inventory_slot_cnt = 28;


// ============================================================
// @section: Camera
// ============================================================

constexpr zcl::t_f32 k_camera_lerp_factor = 0.3f;

struct t_camera {
    zcl::t_v2 position;
};

zcl::t_f32 CameraCalcScale(const zcl::t_v2_i backbuffer_size);
zcl::t_rect_f CameraCalcRect(const t_camera camera, const zcl::t_v2_i backbuffer_size);
zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera camera, const zcl::t_v2_i backbuffer_size);
void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ);
zcl::t_rect_i CameraCalcTilemapRect(const t_camera camera, const zcl::t_v2_i backbuffer_size);

// ============================================================


// ============================================================
// @section: UI
// ============================================================

struct t_world_ui {
    zcl::t_i32 inventory_open;
    zcl::t_i32 inventory_hotbar_slot_selected_index;

    t_item_type_id cursor_held_item_type_id;
    zcl::t_i32 cursor_held_quantity;
};

void WorldUITick(t_world_ui *const ui, zgl::t_input_state *const input_state);
void WorldUIRender(const t_world_ui *const ui, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);

// ============================================================
