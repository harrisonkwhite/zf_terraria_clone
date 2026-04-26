#include "hitboxes.h"

struct t_hitbox_manager {
    zcl::t_list<t_hitbox> hitboxes; // @todo: Might want to reconsider this organization if accounting for caching (colliders are probably better grouped together in the same array). Alternatively, could split based on which ones target NPCs and which ones target the player (though note that there is an overlap in some cases like bombs).
};

t_hitbox_manager *HitboxManagerCreate(const zcl::t_i32 hitbox_limit, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_hitbox_manager>(arena);
    result->hitboxes = zcl::ListCreate<t_hitbox>(hitbox_limit, arena);

    return result;
}

void HitboxSubmit(t_hitbox_manager *const manager, const t_hitbox hitbox) {
    ZCL_ASSERT(hitbox.collider.width >= 0 && hitbox.collider.height >= 0);
    ZCL_ASSERT(hitbox.dmg > 0);

    zcl::ListAppend(&manager->hitboxes, hitbox);
}

zcl::t_array_rdonly<t_hitbox> HitboxesLoadAll(const t_hitbox_manager *const manager) {
    return zcl::ListToArray(&manager->hitboxes);
}

void HitboxesClear(t_hitbox_manager *const manager) {
    zcl::ListClear(&manager->hitboxes);
}
