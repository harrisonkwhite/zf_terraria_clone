#include "world_private.h"

namespace world {
    t_pop_up *PopUpSpawn(t_pop_ups *const pop_ups, const zcl::t_v2 pos, const zcl::t_v2 vel, const t_font_id font_id = ek_font_id_eb_garamond_32) {
        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(pop_ups->activity);

        ZCL_REQUIRE(index != -1);

        pop_ups->buf[index] = {
            .pos = pos,
            .vel = vel,
            .font_id = font_id,
        };

        zcl::BitsetSet(pop_ups->activity, index);

        return &pop_ups->buf[index];
    }
}
