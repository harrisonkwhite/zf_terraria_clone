#pragma once

// ============================================================
// @section: External Forward Declarations

struct t_tilemap_core;

// ==================================================

t_tilemap_core *WorldGen(const zcl::t_v2_i size, zcl::t_rng *const rng, zcl::t_arena *const arena, zcl::t_arena *const temp_arena);
