#include "world_private.h"

namespace world {
    t_pop_up *SpawnPopUp(t_pop_up_manager *const manager, const zcl::t_v2 pos, const zcl::t_v2 vel, const t_font_id font_id) {
        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

        ZCL_REQUIRE(index != -1);

        manager->buf[index] = {
            .pos = pos,
            .vel = vel,
            .font_id = font_id,
        };

        zcl::BitsetSet(manager->activity, index);

        return &manager->buf[index];
    }

    void UpdatePopUps(t_pop_up_manager *const manager) {
        ZCL_BITSET_WALK_ALL_SET (manager->activity, i) {
            const auto pop_up = &manager->buf[i];

            pop_up->pos += pop_up->vel;
            pop_up->vel = zcl::Lerp(pop_up->vel, {}, k_pop_up_lerp_factor);

            if (zcl::CheckNearlyEqual(pop_up->vel, {}, 0.01f)) {
                if (pop_up->death_time < k_pop_up_death_duration) {
                    pop_up->death_time++;
                } else {
                    zcl::BitsetUnset(manager->activity, i);
                }
            }
        }
    }
}
