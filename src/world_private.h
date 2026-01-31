#pragma once

#include "world_public.h"
#include "sprites.h"


// ============================================================
// @section: External Forward Declarations
// ============================================================

struct t_inventory;
struct t_tilemap;

// ============================================================


t_tilemap *WorldGen(zcl::t_rng *const rng, zcl::t_arena *const arena);

constexpr zcl::t_i32 k_player_inventory_slot_cnt = 28;


// ============================================================
// @section: UI
// ============================================================

struct t_world_ui;

t_world_ui *WorldUICreate(zcl::t_arena *const arena);
void WorldUITick(t_world_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state);
void WorldUIRender(const t_world_ui *const ui, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena);

// ============================================================
