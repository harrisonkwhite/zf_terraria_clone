#include "world_private.h"

namespace world {
    constexpr zcl::t_f32 k_player_move_spd = 1.5f;
    constexpr zcl::t_f32 k_player_move_spd_acc = 0.2f;
    constexpr zcl::t_f32 k_player_jump_height = 3.5f;
    constexpr zcl::t_v2 k_player_origin = zcl::k_origin_center;

    struct t_player_meta {
        zcl::t_i32 health_limit;

        t_inventory *inventory;
        zcl::t_i32 inventory_hotbar_slot_selected_index;
    };

    struct t_player_entity {
        zcl::t_i32 health;

        zcl::t_i32 item_use_time;

        zcl::t_v2 pos;
        zcl::t_v2 vel;
        zcl::t_b8 jumping;
    };

    constexpr zcl::t_i32 k_npc_limit = 1024;
    constexpr zcl::t_v2 k_npc_origin = zcl::k_origin_center;
    constexpr zcl::t_i32 k_npc_flash_time_limit = 10;

    struct t_npc {
        zcl::t_v2 pos;

        zcl::t_i32 health;
        zcl::t_i32 health_limit; // This is in here because it could be randomized even across NPCs of the same type.

        zcl::t_i32 flash_time;

        t_npc_type_id type_id;

        union {
            struct {
                zcl::t_v2 vel;
            } slime;
        } type_data;
    };

    struct t_npc_manager {
        zcl::t_static_array<t_npc, k_npc_limit> buf;
        zcl::t_static_bitset<k_npc_limit> activity;
        zcl::t_static_array<zcl::t_i32, k_npc_limit> versions;
    };

    t_player_meta *PlayerCreateMeta(zcl::t_arena *const arena) {
        const auto result = zcl::ArenaPush<t_player_meta>(arena);

        result->health_limit = 100;

        result->inventory = InventoryCreate({7, 4}, arena);
        InventoryAdd(result->inventory, ek_item_type_id_copper_pickaxe, 1);

        return result;
    }

    t_player_entity *PlayerCreateEntity(const t_player_meta *const player_meta, const t_tilemap *const tilemap, zcl::t_arena *const arena) {
        const auto result = zcl::ArenaPush<t_player_entity>(arena);

        result->health = player_meta->health_limit;

        const zcl::t_rect_f player_collider = PlayerGetCollider(result->pos);

        const zcl::t_f32 player_x = (k_tilemap_size.x * k_tile_size) / 2.0f;

        result->pos = TilemapMoveContact({player_x, -player_collider.height * (1.0f - k_player_origin.y)}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(player_collider), k_player_origin, tilemap);

        return result;
    }

    t_inventory *PlayerGetInventory(t_player_meta *const player_meta) {
        return player_meta->inventory;
    }

    zcl::t_v2 PlayerGetPos(t_player_entity *const player_entity) {
        return player_entity->pos;
    }

    static zcl::t_v2 PlayerGetColliderSize(const zcl::t_v2 pos) {
        return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect));
    }

    zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 pos) {
        return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect)), k_player_origin);
    }

    static zcl::t_b8 PlayerCheckGrounded(const zcl::t_v2 player_entity_pos, const t_tilemap *const tilemap) {
        const zcl::t_rect_f collider_below = zcl::RectCreateTranslated(PlayerGetCollider(player_entity_pos), {0.0f, 1.0f});
        return TilemapCheckCollision(tilemap, collider_below);
    }

    void PlayerProcessMovement(t_player_entity *const player_entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state) {
        const zcl::t_f32 move_axis = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d) - zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);

        const zcl::t_f32 move_spd_targ = move_axis * k_player_move_spd;

        if (player_entity->vel.x < move_spd_targ) {
            player_entity->vel.x += zcl::CalcMin(move_spd_targ - player_entity->vel.x, k_player_move_spd_acc);
        } else if (player_entity->vel.x > move_spd_targ) {
            player_entity->vel.x -= zcl::CalcMin(player_entity->vel.x - move_spd_targ, k_player_move_spd_acc);
        }

        player_entity->vel.y += k_gravity;

        const zcl::t_b8 grounded = PlayerCheckGrounded(player_entity->pos, tilemap);

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

        TilemapProcessCollisions(tilemap, &player_entity->pos, &player_entity->vel, PlayerGetColliderSize(player_entity->pos), k_player_origin);

        player_entity->pos += player_entity->vel;
    }

    void PlayerProcessInventoryHotbarUpdates(t_player_meta *const player_meta, const zgl::t_input_state *const input_state) {
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

    void PlayerProcessItemUsage(const t_player_meta *const player_meta, t_player_entity *const player_entity, const t_tilemap *const tilemap, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        if (player_entity->item_use_time > 0) {
            player_entity->item_use_time--;
        } else {
            const t_inventory_slot hotbar_slot_selected = InventoryGet(player_meta->inventory, {player_meta->inventory_hotbar_slot_selected_index, 0});

            if (hotbar_slot_selected.quantity > 0) {
                const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

#if 0
            const zcl::t_b8 item_use = g_item_types[item_type_id].use_hold ? zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left) : zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left);

            if (item_use) {
                const zcl::t_b8 item_use_success = k_item_type_use_funcs[item_type_id]({
                    .world = world,
                    .cursor_pos = zgl::CursorGetPos(input_state),
                    .screen_size = screen_size,
                    .temp_arena = temp_arena,
                });

                if (item_use_success) {
                    if (g_item_types[item_type_id].use_consume) {
                        InventoryRemoveAt(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index, 1);
                    }

                    player_entity->item_use_time = g_item_types[hotbar_slot_selected.item_type_id].use_time;
                }
            }
#endif
            }
        }
    }

    void PlayerRender(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets) {
        SpriteRender(ek_sprite_id_player, rc, assets, {player_entity->pos.x, player_entity->pos.y}, k_player_origin);
    }

    t_npc_manager *NPCManagerCreate(zcl::t_arena *const arena) {
        return zcl::ArenaPush<t_npc_manager>(arena);
    }

    t_npc_id NPCSpawn(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id) {
        const zcl::t_i32 index = zcl::BitsetFindFirstUnset(manager->activity);

        if (index == -1) {
            ZCL_FATAL();
        }

        zcl::BitsetSet(manager->activity, index);

        manager->buf[index] = {
            .pos = pos,
            .type_id = type_id,
        };

        switch (type_id) {
            case ek_npc_type_id_slime: {
                break;
            }

            case ekm_npc_type_id_cnt: {
                ZCL_UNREACHABLE();
            }
        }

        return {index, manager->versions[index]};
    }

    void NPCHurt(t_npc_manager *const manager, const t_npc_id id, const zcl::t_i32 damage) {
        ZCL_ASSERT(NPCCheckExists(manager, id));

        const auto npc = &manager->buf[id.index];
        npc->flash_time = k_npc_flash_time_limit;
    }

    zcl::t_b8 NPCCheckExists(const t_npc_manager *const manager, const t_npc_id id) {
        return zcl::BitsetCheckSet(manager->activity, id.index) && id.version == manager->versions[id.index];
    }

    static zcl::t_v2 NPCGetColliderSize(const zcl::t_v2 pos, const t_npc_type_id type_id) {
        return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect));
    }

    zcl::t_rect_f NPCGetCollider(const zcl::t_v2 pos, const t_npc_type_id type_id) {
        switch (type_id) {
            case ek_npc_type_id_slime:
                return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_npc_slime].src_rect)), k_npc_origin);

            case ekm_npc_type_id_cnt:
                ZCL_UNREACHABLE();
        }

        ZCL_UNREACHABLE();
    }

    void NPCsProcessAI(t_npc_manager *const npcs, const t_tilemap *const tilemap) {
        ZCL_BITSET_WALK_ALL_SET (npcs->activity, i) {
            const auto npc = &npcs->buf[i];

            switch (npc->type_id) {
                case ek_npc_type_id_slime: {
                    const auto slime = &npc->type_data.slime;

                    slime->vel.y += k_gravity;

                    TilemapProcessCollisions(tilemap, &npc->pos, &slime->vel, NPCGetColliderSize(npc->pos, npc->type_id), k_npc_origin);

                    npc->pos += slime->vel;

                    break;
                }

                case ekm_npc_type_id_cnt: {
                    ZCL_UNREACHABLE();
                }
            }
        }
    }

    void NPCsProcessDeaths(t_npc_manager *const npcs) {
        ZCL_BITSET_WALK_ALL_SET (npcs->activity, i) {
            const auto npc = &npcs->buf[i];

            ZCL_ASSERT(npc->health >= 0);

            if (npc->health == 0) {
                zcl::BitsetUnset(npcs->activity, i);
            }
        }
    }

    void NPCsRender(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets) {
        ZCL_BITSET_WALK_ALL_SET (manager->activity, i) {
            const auto npc = &manager->buf[i];

            switch (npc->type_id) {
                case ek_npc_type_id_slime: {
                    SpriteRender(ek_sprite_id_npc_slime, rc, assets, npc->pos, zcl::k_origin_center);
                    break;
                }

                case ekm_npc_type_id_cnt: {
                    ZCL_UNREACHABLE();
                }
            }
        }
    }

#if 0
    void ProcessPlayerAndNPCCollisions(const t_player_entity *const player_entity, const t_npc_manager *const npc_manager) {
        // const zcl::t_rect_f player_collider = PlayerColliderCreate();
    }
#endif
}
