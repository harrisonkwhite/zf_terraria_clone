#include "world.h"

struct t_world {
};

t_world *WorldInit(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_world>(arena);
}

void WorldTick(t_world *const world) {
}

void WorldRender(t_world *const world) {
}
