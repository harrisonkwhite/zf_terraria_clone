#pragma once

struct t_world;

t_world *WorldCreate(zcl::t_arena *const arena);
void WorldTick(t_world *const world, const zgl::t_input_state *const input_state);
void WorldRender(t_world *const world);
