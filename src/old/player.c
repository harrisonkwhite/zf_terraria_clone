#include "game.h"

#include <stdio.h>

#define PLAYER_MOVE_SPD 1.5f
#define PLAYER_MOVE_SPD_ACC 0.2f
#define PLAYER_JUMP_HEIGHT 3.5f

#define PLAYER_INV_DUR 30
#define PLAYER_INV_ALPHA_LOW 0.5f
#define PLAYER_INV_ALPHA_HIGH 0.7f

static inline bool IsPlayerGrounded(const s_v2 player_pos, const t_tilemap_activity* const tm_activity) {
    const s_rect below_collider = RectTranslated(PlayerCollider(player_pos), (s_v2){0.0f, 1.0f});
    return TileCollisionCheck(tm_activity, below_collider);
}

void InitPlayer(s_player* const player, const t_s32 hp_max, const t_tilemap_activity* const tm_activity) {
    assert(player && IS_ZERO(*player));
    assert(hp_max >= 0);
    assert(tm_activity);

    player->pos.x = TILE_SIZE * TILEMAP_WIDTH * 0.5f;
    MakeContactWithTilemap(&player->pos, ek_cardinal_dir_down, PlayerColliderSize(), PLAYER_ORIGIN, tm_activity);

    player->hp = hp_max;
}

bool UpdatePlayer(s_world* const world, const s_input_context* const input_context) {
    assert(!world->player.killed);

    //
    // Movement
    //
    const t_r32 move_axis = IsKeyDown(input_context, ek_key_code_d) - IsKeyDown(input_context, ek_key_code_a);
    const t_r32 move_spd_dest = move_axis * PLAYER_MOVE_SPD;

    if (world->player.vel.x < move_spd_dest) {
        world->player.vel.x += MIN(move_spd_dest - world->player.vel.x, PLAYER_MOVE_SPD_ACC);
    } else if (world->player.vel.x > move_spd_dest) {
        world->player.vel.x -= MIN(world->player.vel.x - move_spd_dest, PLAYER_MOVE_SPD_ACC);
    }

    world->player.vel.y += GRAVITY;

    const bool grounded = IsPlayerGrounded(world->player.pos, &world->core.tilemap_core.activity);

    if (grounded) {
        world->player.jumping = false;
    }

    if (!world->player.jumping) {
        if (grounded && IsKeyPressed(input_context, ek_key_code_space)) {
            world->player.vel.y = -PLAYER_JUMP_HEIGHT;
            world->player.jumping = true;
        }
    } else {
        if (world->player.vel.y < 0.0f && !IsKeyDown(input_context, ek_key_code_space)) {
            world->player.vel.y = 0.0f;
        }
    }

    ProcTileCollisions(&world->player.pos, &world->player.vel, PlayerColliderSize(), PLAYER_ORIGIN, &world->core.tilemap_core.activity);

    world->player.pos = V2Sum(world->player.pos, world->player.vel);

    //
    // NPC Collisions
    //
    if (world->player.invinc_time <= 0) {
        const s_rect player_collider = PlayerCollider(world->player.pos);

        for (t_s32 i = 0; i < NPC_LIMIT; i++) {
            const s_npc* const npc = &world->npcs.buf[i]; // NOTE: Constant probably temporarily.

            if (!IsNPCActivityBitSet(&world->npcs.activity, i)) {
                continue;
            }

            const s_npc_type* const npc_type = &g_npc_types[npc->type];

            if (npc_type->contact_dmg == 0) {
                continue;
            }

            const s_rect npc_collider = NPCCollider(npc->pos, npc->type);

            if (DoRectsInters(player_collider, npc_collider)) {
                const s_v2 dir = V2Dir(npc->pos, world->player.pos);
                const s_v2 kb = {dir.x * npc_type->contact_kb, dir.y * npc_type->contact_kb};

                if (!HurtPlayer(world, npc_type->contact_dmg, kb)) {
                    return false;
                }

                break;
            }
        }
    }

    //
    // Updating Timers
    //
    if (world->player.invinc_time > 0) {
        world->player.invinc_time--;
    }

    if (world->player.flash_time > 0) {
        world->player.flash_time--;
    }

    //
    // Death
    //
    if (world->player.hp == 0) {
        // TODO: Do some magic stuff!
        world->player.killed = true;
    }

    return true;
}

void RenderPlayer(const s_player* const player, const s_rendering_context* const rendering_context, const s_texture_group* const textures, const s_surface* const temp_surf) {
    assert(!player->killed);

    if (player->flash_time > 0) {
        SetSurface(rendering_context, temp_surf);
        Clear(rendering_context, (u_v4){0});
    }

    t_r32 alpha = 1.0f;

    if (player->invinc_time > 0) {
        alpha = player->invinc_time % 2 == 0 ? PLAYER_INV_ALPHA_LOW : PLAYER_INV_ALPHA_HIGH;
    }

    RenderSprite(rendering_context, ek_sprite_player, textures, player->pos, PLAYER_ORIGIN, (s_v2){1.0f, 1.0f}, 0.0f, (u_v4){1.0f, 1.0f, 1.0f, alpha});

    if (player->flash_time > 0) {
        UnsetSurface(rendering_context);

        SetSurfaceShaderProg(rendering_context, &rendering_context->basis->builtin_shader_progs, ek_builtin_shader_prog_surface_blend);

        SetSurfaceShaderProgUniform(rendering_context, "u_col", (s_shader_prog_uniform_value){.type = ek_shader_prog_uniform_value_type_v3, .as_v3 = WHITE.rgb});

        assert(player->flash_time >= 0 && player->flash_time <= PLAYER_HURT_FLASH_TIME);
        SetSurfaceShaderProgUniform(rendering_context, "u_intensity", (s_shader_prog_uniform_value){.type = ek_shader_prog_uniform_value_type_r32, .as_r32 = (float)player->flash_time / PLAYER_HURT_FLASH_TIME});

        RenderSurface(rendering_context, temp_surf, (s_v2){0}, V2_ONE, true);
    }
}

bool HurtPlayer(s_world* const world, const t_s32 dmg, const s_v2 kb) {
    assert(dmg > 0);
    assert(world->player.invinc_time == 0);

    world->player.hp = MAX(world->player.hp - dmg, 0);
    world->player.vel = kb;
    world->player.jumping = false;
    world->player.invinc_time = PLAYER_INV_DUR;
    world->player.flash_time = PLAYER_HURT_FLASH_TIME;

    s_popup_text* const dmg_popup = SpawnPopupText(world, world->player.pos, RandRange(DMG_POPUP_TEXT_VEL_Y_MIN, DMG_POPUP_TEXT_VEL_Y_MAX));

    if (!dmg_popup) {
        return false;
    }

    snprintf(dmg_popup->str, sizeof(dmg_popup->str), "%d", -dmg);

    return true;
}
