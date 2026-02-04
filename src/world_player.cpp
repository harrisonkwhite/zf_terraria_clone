#include "world_private.h"

#include "inventories.h"

namespace world {
    t_player_meta CreatePlayerMeta(zcl::t_arena *const arena) {
        const auto inventory = InventoryCreate({7, 4}, arena);
        InventoryAdd(inventory, ek_item_type_id_copper_pickaxe, 1);

        return {
            .health_limit = 100,
            .inventory = inventory,
        };
    }

    static zcl::t_v2 GetPlayerColliderSize() {
        return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect));
    }

    t_player_entity CreatePlayerEntity(const t_player_meta *const player_meta, const t_tilemap *const tilemap) {
        const zcl::t_i32 health = player_meta->health_limit;

        const zcl::t_v2 collider_size = GetPlayerColliderSize();
        const zcl::t_f32 x = (k_tilemap_size.x * k_tile_size) / 2.0f;
        const zcl::t_v2 pos = TilemapMoveContact({x, -collider_size.y * (1.0f - k_player_origin.y)}, zcl::ek_cardinal_direction_down, collider_size, k_player_origin, tilemap);

        return {
            .active = true,
            .health = health,
            .pos = pos,
        };
    }

    zcl::t_rect_f GetPlayerCollider(const zcl::t_v2 pos) {
        return ColliderCreate(pos, GetPlayerColliderSize(), k_player_origin);
    }

    static zcl::t_b8 CheckPlayerGrounded(const zcl::t_v2 player_entity_pos, const t_tilemap *const tilemap) {
        const zcl::t_rect_f collider_below = zcl::RectCreateTranslated(GetPlayerCollider(player_entity_pos), {0.0f, 1.0f});
        return TilemapCheckCollision(tilemap, collider_below);
    }

    void UpdatePlayerTimers(t_player_entity *const player_entity) {
        ZCL_ASSERT(player_entity->active);

        if (player_entity->invincible_time > 0) {
            player_entity->invincible_time--;
        }

        if (player_entity->flash_time > 0) {
            player_entity->flash_time--;
        }
    }

    void PlayerUpdateMovement(t_player_entity *const player_entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state) {
        ZCL_ASSERT(player_entity->active);

        const zcl::t_f32 move_axis = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d) - zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);

        const zcl::t_f32 move_spd_targ = move_axis * k_player_move_spd;

        if (player_entity->vel.x < move_spd_targ) {
            player_entity->vel.x += zcl::CalcMin(move_spd_targ - player_entity->vel.x, k_player_move_spd_acc);
        } else if (player_entity->vel.x > move_spd_targ) {
            player_entity->vel.x -= zcl::CalcMin(player_entity->vel.x - move_spd_targ, k_player_move_spd_acc);
        }

        player_entity->vel.y += k_gravity;

        const zcl::t_b8 grounded = CheckPlayerGrounded(player_entity->pos, tilemap);

        if (grounded) {
            player_entity->jumping = false;
        }

        if (!player_entity->jumping) {
            if (grounded && zgl::KeyCheckPressed(input_state, zgl::ek_key_code_space)) {
                player_entity->vel.y = -k_player_jump_height;
                player_entity->jumping = true;
            }
        } else {
            if (player_entity->vel.y < 0.0f && !zgl::KeyCheckDown(input_state, zgl::ek_key_code_space)) {
                player_entity->vel.y = 0.0f;
            }
        }

        TilemapProcessCollisions(tilemap, &player_entity->pos, &player_entity->vel, GetPlayerColliderSize(), k_player_origin);

        player_entity->pos += player_entity->vel;
    }

    void ProcessPlayerInventoryHotbarUpdates(t_player_meta *const player_meta, const zgl::t_input_state *const input_state) {
        const zcl::t_i32 inventory_width = InventoryGetSize(player_meta->inventory).x;

        for (zcl::t_i32 i = 0; i < inventory_width; i++) {
            if (zgl::KeyCheckPressed(input_state, static_cast<zgl::t_key_code>(zgl::ek_key_code_1 + i))) {
                player_meta->inventory_hotbar_slot_selected_index = i;
                break;
            }
        }

        const zcl::t_v2 scroll_offs = zgl::ScrollGetOffset(input_state);

        if (scroll_offs.y != 0.0f) {
            player_meta->inventory_hotbar_slot_selected_index += round(scroll_offs.y);
            player_meta->inventory_hotbar_slot_selected_index = zcl::Wrap(player_meta->inventory_hotbar_slot_selected_index, 0, inventory_width);
        }
    }

    void ProcessPlayerItemUsage(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
        ZCL_ASSERT(world->player_entity.active);

        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        if (world->player_entity.item_use_time > 0) {
            world->player_entity.item_use_time--;
        } else {
            const t_inventory_slot hotbar_slot_selected = InventoryGet(world->player_meta.inventory, {world->player_meta.inventory_hotbar_slot_selected_index, 0});

            if (hotbar_slot_selected.quantity > 0) {
                const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

                const zcl::t_b8 item_use = g_item_types[item_type_id].use_hold ? zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left) : zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left);

                if (item_use) {
                    const t_item_type_use_func_context item_use_func_context = {
                        .cursor_pos = cursor_pos,
                        .screen_size = screen_size,
                        .temp_arena = temp_arena,
                        .tilemap = world->tilemap,
                        .player_meta = &world->player_meta,
                        .player_entity = &world->player_entity,
                        .npc_manager = &world->npc_manager,
                        .item_drop_manager = &world->item_drop_manager,
                        .camera = world->camera,
                        .pop_up_manager = &world->pop_up_manager,
                        .rng = world->rng,
                    };

                    const zcl::t_b8 item_use_success = g_item_type_use_funcs[item_type_id](item_use_func_context);

                    if (item_use_success) {
                        if (g_item_types[item_type_id].use_consume) {
                            InventoryRemoveAt(world->player_meta.inventory, {world->player_meta.inventory_hotbar_slot_selected_index, 0}, 1);
                        }

                        world->player_entity.item_use_time = g_item_types[hotbar_slot_selected.item_type_id].use_time;
                    }
                }
            }
        }
    }

    void ProcessPlayerDeath(t_player_meta *const player_meta, t_player_entity *const player_entity) {
        ZCL_ASSERT(player_entity->active);

        if (player_entity->health == 0) {
            player_entity->active = false;
            player_meta->respawn_time = k_player_respawn_duration;
        }
    }

    void HurtPlayer(t_player_entity *const player_entity, const zcl::t_i32 damage, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng) {
        ZCL_ASSERT(player_entity->active);
        ZCL_ASSERT(damage > 0);

        if (player_entity->invincible_time > 0) {
            return;
        }

        player_entity->health -= zcl::CalcMin(player_entity->health, damage);
        player_entity->invincible_time = k_player_invincible_duration;
        player_entity->flash_time = k_player_flash_duration;

        SpawnPopUpDamage(pop_up_manager, player_entity->pos, damage, rng);
    }

    void RenderPlayer(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets) {
        ZCL_ASSERT(player_entity->active);

        if (player_entity->flash_time > 0) {
            ZCL_ASSERT(player_entity->flash_time <= k_player_flash_duration);
            const zcl::t_f32 flash_time_perc = static_cast<zcl::t_f32>(player_entity->flash_time) / k_player_flash_duration;

            const auto blend_uniform = zgl::RendererGetBuiltinUniform(rc.basis, zgl::ek_renderer_builtin_uniform_id_blend);

            zgl::UniformSetV4(rc.gfx_ticket, blend_uniform, {1.0f, 1.0f, 1.0f, flash_time_perc});

            const auto blend_shader_prog = zgl::RendererGetBuiltinShaderProg(rc.basis, zgl::ek_renderer_builtin_shader_prog_id_blend);
            zgl::RendererSetShaderProg(rc, blend_shader_prog);
        }

        SpriteRender(ek_sprite_id_player, rc, assets, player_entity->pos, k_player_origin);

        if (player_entity->flash_time > 0) {
            zgl::RendererSetShaderProg(rc, nullptr);
        }
    }
}
