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

    t_pop_up *SpawnPopUpDamage(t_pop_up_manager *const manager, const zcl::t_v2 pos, const zcl::t_i32 damage, zcl::t_rng *const rng) {
        ZCL_ASSERT(damage > 0);

        const zcl::t_f32 vel_dir = (-zcl::k_pi / 2.0f) + zcl::RandGenF32InRange(rng, -(zcl::k_pi / 16.0f), zcl::k_pi / 16.0f);
        const zcl::t_v2 vel = zcl::CalcLengthDir(zcl::RandGenF32InRange(rng, 4.0f, 5.0f), vel_dir);

        const auto pop_up = SpawnPopUp(manager, pos, vel);
        zcl::t_byte_stream str_bytes_stream = zcl::ByteStreamCreate(pop_up->str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("-%"), damage);
        pop_up->str_byte_cnt = zcl::ByteStreamGetWritten(&str_bytes_stream).len;

        return pop_up;
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
