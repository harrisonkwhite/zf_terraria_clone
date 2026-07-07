#include "npcs.h"

#include "assets.h"
#include "player.h"
#include "tiles.h"
#include "hitboxes.h"
#include "pop_ups.h"
#include "item_drops.h"
#include "audio_helpers.h"
#include "stray.h"

constexpr zcl::t_i32 k_npc_limit = 1024;
constexpr zcl::t_i32 k_npc_flash_duration = 10;

constexpr zcl::t_f32 k_npc_slime_jump_height_min_incl = 2.5f;
constexpr zcl::t_f32 k_npc_slime_jump_height_max_excl = 4.0f;
constexpr zcl::t_f32 k_npc_slime_jump_hor_spd_min_incl = 1.5f;
constexpr zcl::t_f32 k_npc_slime_jump_hor_spd_max_excl = 2.0f;
constexpr zcl::t_f32 k_npc_slime_jump_hor_spd_lerp_factor = 0.2f;
constexpr zcl::t_i32 k_npc_slime_jump_break_min_incl = 30;
constexpr zcl::t_i32 k_npc_slime_jump_break_max_excl = 90;

struct t_npc {
    zcl::t_i32 health;

    zcl::t_v2 pos;

    t_npc_type_id type_id;

    union {
        struct {
            zcl::t_v2 vel;
            zcl::t_i32 vel_x_axis_targ;
            zcl::t_f32 jump_hor_spd;
            zcl::t_i32 jump_break;
        } slime;
    } type_data;

    zcl::t_i32 flash_time;
};

struct t_npc_manager {
    zcl::t_static_array<t_npc, k_npc_limit> buf;
    zcl::t_static_bitset<k_npc_limit> activity;
    zcl::t_static_array<zcl::t_i32, k_npc_limit> versions;
};

t_npc_manager *NPCManagerCreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_npc_manager>(arena);
}

t_npc_id NPCSpawn(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id, zcl::t_rng *const rng) {
    const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

    if (index == -1) {
        ZCL_FATAL();
    }

    zcl::BitsetSet(manager->activity, index);

    const auto npc = &manager->buf[index];

    *npc = {
        .health = g_npc_types[type_id].health_limit,
        .pos = pos,
        .type_id = type_id,
    };

    switch (type_id) {
        case ek_npc_type_id_slime: {
            npc->type_data.slime.jump_break = zcl::RandGenI32InRange(rng, k_npc_slime_jump_break_min_incl, k_npc_slime_jump_break_max_excl);
            break;
        }

        case ekm_npc_type_id_cnt: {
            ZCL_UNREACHABLE();
        }
    }

    return {index, manager->versions[index]};
}

zcl::t_b8 NPCCheckExists(const t_npc_manager *const manager, const t_npc_id id) {
    return zcl::BitsetCheckSet(manager->activity, id.index) && id.version == manager->versions[id.index];
}

zcl::t_v2 NPCGetPosition(const t_npc_manager *const manager, const t_npc_id id) {
    ZCL_ASSERT(NPCCheckExists(manager, id));
    return manager->buf[id.index].pos;
}

zcl::t_i32 NPCGetHealth(const t_npc_manager *const manager, const t_npc_id id) {
    ZCL_ASSERT(NPCCheckExists(manager, id));
    return manager->buf[id.index].health;
}

t_npc_type_id NPCGetTypeID(const t_npc_manager *const manager, const t_npc_id id) {
    ZCL_ASSERT(NPCCheckExists(manager, id));
    return manager->buf[id.index].type_id;
}

zcl::t_rect_f NPCGetCollider(const zcl::t_v2 pos, const t_npc_type_id type_id) {
    switch (type_id) {
        case ek_npc_type_id_slime: {
            return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect)), k_npc_origin);
        }

        case ekm_npc_type_id_cnt: {
            ZCL_UNREACHABLE();
        }
    }

    ZCL_UNREACHABLE();
}

zcl::t_v2 NPCGetColliderSize(const t_npc_type_id type_id) {
    return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect));
}

void NPCsProcessAIs(t_npc_manager *const manager, const zcl::t_f32 gravity, const t_player_entity *const player_entity, const t_tilemap *const tilemap, zcl::t_rng *const rng) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

        const auto npc = &manager->buf[i];

        switch (npc->type_id) {
            case ek_npc_type_id_slime: {
                const auto slime = &npc->type_data.slime;

                slime->vel.y += gravity;

                zcl::t_f32 vel_x_targ = 0.0f;

                if (CheckOnGround(NPCGetCollider(npc->pos, ek_npc_type_id_slime), tilemap)) {
                    slime->vel_x_axis_targ = 0;

                    if (slime->jump_break > 0) {
                        slime->jump_break--;
                    } else {
                        slime->vel.y = -zcl::RandGenF32InRange(rng, k_npc_slime_jump_height_min_incl, k_npc_slime_jump_height_max_excl);
                        slime->vel_x_axis_targ = zcl::CalcSign(PlayerGetPosition(player_entity).x - npc->pos.x);
                        slime->jump_hor_spd = zcl::RandGenF32InRange(rng, k_npc_slime_jump_hor_spd_min_incl, k_npc_slime_jump_hor_spd_max_excl);
                        slime->jump_break = zcl::RandGenI32InRange(rng, k_npc_slime_jump_break_min_incl, k_npc_slime_jump_break_max_excl);
                    }
                } else {
                    vel_x_targ = slime->jump_hor_spd * slime->vel_x_axis_targ;
                }

                slime->vel.x = zcl::Lerp(slime->vel.x, vel_x_targ, k_npc_slime_jump_hor_spd_lerp_factor);

                ProcessTilemapCollisions(&npc->pos, &slime->vel, NPCGetColliderSize(npc->type_id), k_npc_origin, tilemap);

                npc->pos += slime->vel;

                break;
            }

            case ekm_npc_type_id_cnt: {
                ZCL_UNREACHABLE();
            }
        }

        if (npc->flash_time > 0) {
            npc->flash_time--;
        }
    }
}

void NPCsSubmitHitboxes(const t_npc_manager *const npc_manager, t_hitbox_manager *const hitbox_manager) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(npc_manager->activity, i)) {
            continue;
        }

        const auto npc = &npc_manager->buf[i];
        const auto npc_type = &g_npc_types[npc->type_id];

        if (!npc_type->touch_hurt) {
            continue;
        }

        const t_hitbox hitbox = {
            .collider = NPCGetCollider(npc->pos, npc->type_id),
            .dmg = npc_type->touch_hurt_damage,
            .flags = ek_hitbox_flag_hurt_player,
        };

        HitboxSubmit(hitbox_manager, hitbox);
    }
}

static void NPCHurt(t_npc *const npc, const zcl::t_i32 damage, t_pop_up_manager *const pop_up_manager, const zgl::t_audio_ticket_mut audio_ticket, const t_options *const options, const t_assets *const assets, zcl::t_rng *const rng) {
    npc->health = zcl::CalcMax(npc->health - damage, 0);
    npc->flash_time = k_npc_flash_duration;
    PopUpSpawnDamage(pop_up_manager, npc->pos, damage, rng);
    SoundFireAndForgetWithOptions(audio_ticket, SoundTypeGet(assets, ek_sound_type_id_npc_hurt), options);
}

zcl::t_b8 NPCsCheckCollision(const t_npc_manager *const npc_manager, const zcl::t_rect_f collider) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(npc_manager->activity, i)) {
            continue;
        }

        const auto npc = &npc_manager->buf[i];
        const auto npc_collider = NPCGetCollider(npc->pos, npc->type_id);

        if (zcl::CheckInters(collider, npc_collider)) {
            return true;
        }
    }

    return false;
}

void NPCsProcessHitboxCollisions(t_npc_manager *const npc_manager, const zcl::t_array_rdonly<t_hitbox> hitboxes, t_pop_up_manager *const pop_up_manager, const zgl::t_audio_ticket_mut audio_ticket, const t_options *const options, const t_assets *const assets, zcl::t_rng *const rng) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(npc_manager->activity, i)) {
            continue;
        }

        const auto npc = &npc_manager->buf[i];
        const auto npc_collider = NPCGetCollider(npc->pos, npc->type_id);

        for (zcl::t_i32 j = 0; j < hitboxes.len; j++) {
            const auto hitbox = &hitboxes[j];

            // @todo: Pull this check out of the loop.
            if (!(hitbox->flags & ek_hitbox_flag_hurt_npcs)) {
                continue;
            }

            if (zcl::CheckInters(npc_collider, hitbox->collider)) {
                NPCHurt(npc, hitbox->dmg, pop_up_manager, audio_ticket, options, assets, rng);
            }
        }
    }
}

void NPCsProcessDeaths(t_npc_manager *const manager, t_item_drop_manager *const item_drop_manager, const zgl::t_audio_ticket_mut audio_ticket, const t_options *const options, const t_assets *const assets, zcl::t_rng *const rng) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

        const auto npc = &manager->buf[i];

        ZCL_ASSERT(npc->health >= 0);

        if (npc->health == 0) {
            switch (npc->type_id) {
                case ek_npc_type_id_slime: {
                    ItemDropSpawn(item_drop_manager, npc->pos, ek_item_type_id_gel, zcl::RandGenI32InRange(rng, 2, 4));
                    break;
                }

                default: {
                    ZCL_UNREACHABLE();
                }
            }

            SoundFireAndForgetWithOptions(audio_ticket, SoundTypeGet(assets, ek_sound_type_id_npc_die), options);

            zcl::BitsetUnset(manager->activity, i);
        }
    }
}

void NPCsRender(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets) {
    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

        const auto npc = &manager->buf[i];

        if (npc->flash_time > 0) {
            ZCL_ASSERT(npc->flash_time <= k_npc_flash_duration);
            const zcl::t_f32 flash_time_perc = static_cast<zcl::t_f32>(npc->flash_time) / k_npc_flash_duration;

            const auto blend_uniform = zgl::RendererGetBuiltinUniform(rc.basis, zgl::ek_renderer_builtin_uniform_id_blend);

            zgl::UniformSetV4(rc.gfx_ticket, blend_uniform, {1.0f, 1.0f, 1.0f, flash_time_perc});

            const auto blend_shader_prog = zgl::RendererGetBuiltinShaderProg(rc.basis, zgl::ek_renderer_builtin_shader_prog_id_blend);
            zgl::RendererSetShaderProg(rc, blend_shader_prog);
        }

        switch (npc->type_id) {
            case ek_npc_type_id_slime: {
                SpriteRender(ek_sprite_id_npc_slime, rc, assets, npc->pos, k_npc_origin);
                break;
            }

            case ekm_npc_type_id_cnt: {
                ZCL_UNREACHABLE();
            }
        }

        if (npc->flash_time > 0) {
            zgl::RendererSetShaderProg(rc, nullptr);
        }
    }
}

zcl::t_array_mut<t_npc_id> NPCsLoad(const t_npc_manager *const manager, zcl::t_arena *const arena) {
    const zcl::t_i32 npc_cnt = zcl::BitsetCountSet(manager->activity);

    const auto result = zcl::ArenaPushArray<t_npc_id>(arena, npc_cnt);

    for (zcl::t_i32 i = 0, result_index = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(manager->activity, i)) {
            continue;
        }

        result[result_index] = {i, manager->versions[i]};
        result_index++;
    }

    return result;
}

zcl::t_i32 NPCsGetCount(const t_npc_manager *const manager) {
    return zcl::BitsetCountSet(manager->activity);
}
