#include "world_private.h"

#include "inventories.h"
#include "tilemaps.h"

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
        return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect)), k_player_origin);
    }

    static zcl::t_b8 CheckPlayerGrounded(const zcl::t_v2 player_entity_pos, const t_tilemap *const tilemap) {
        const zcl::t_rect_f collider_below = zcl::RectCreateTranslated(GetPlayerCollider(player_entity_pos), {0.0f, 1.0f});
        return TilemapCheckCollision(tilemap, collider_below);
    }

    void ProcessPlayerMovement(t_player_entity *const player_entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state) {
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

    void ProcessPlayerItemUsage(const t_player_meta *const player_meta, t_player_entity *const player_entity, const t_tilemap *const tilemap, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
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

    void RenderPlayer(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets) {
        SpriteRender(ek_sprite_id_player, rc, assets, {player_entity->pos.x, player_entity->pos.y}, k_player_origin);
    }
}
