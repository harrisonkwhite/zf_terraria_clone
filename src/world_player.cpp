#include "world_private.h"

constexpr zcl::t_f32 k_player_entity_move_spd = 1.5f;
constexpr zcl::t_f32 k_player_entity_move_spd_acc = 0.2f;
constexpr zcl::t_f32 k_player_entity_jump_height = 3.5f;
constexpr zcl::t_v2 k_player_entity_origin = zcl::k_origin_center;

struct t_player_entity {
    zcl::t_v2 pos;
    zcl::t_v2 vel;
    zcl::t_b8 jumping;

    zcl::t_i32 item_use_time;

    zcl::t_i32 player_health;
};

struct t_player_meta {
    zcl::t_i32 player_health_limit;
    t_inventory *player_inventory;
};

t_player_entity *PlayerEntityCreate(const zcl::t_v2 pos, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_player_entity>(arena);
    result->pos = pos;

    // result->pos = MakeContactWithTilemap({}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(PlayerEntityGetCollider(result->pos)), k_player_entity_origin, tilemap);

    return result;
}

zcl::t_v2 PlayerGetPos(t_player_entity *const entity) {
    return entity->pos;
}

static zcl::t_v2 PlayerEntityGetColliderSize(const zcl::t_v2 pos) {
    return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect));
}

zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 pos) {
    return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect)), k_player_entity_origin);
}

static zcl::t_b8 PlayerEntityCheckGrounded(const zcl::t_v2 entity_pos, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_below = zcl::RectCreateTranslated(PlayerGetCollider(entity_pos), {0.0f, 1.0f});
    return TilemapCheckCollision(tilemap, collider_below);
}

void PlayerProcessMovement(t_player_entity *const entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state) {
    const zcl::t_f32 move_axis = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d) - zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);

    const zcl::t_f32 move_spd_targ = move_axis * k_player_entity_move_spd;

    if (entity->vel.x < move_spd_targ) {
        entity->vel.x += zcl::CalcMin(move_spd_targ - entity->vel.x, k_player_entity_move_spd_acc);
    } else if (entity->vel.x > move_spd_targ) {
        entity->vel.x -= zcl::CalcMin(entity->vel.x - move_spd_targ, k_player_entity_move_spd_acc);
    }

    entity->vel.y += k_gravity;

    const zcl::t_b8 grounded = PlayerEntityCheckGrounded(entity->pos, tilemap);

    if (grounded) {
        entity->jumping = false;
    }

    if (!entity->jumping) {
        if (grounded && zgl::KeyCheckPressed(input_state, zgl::ek_key_code_space)) {
            entity->vel.y = -k_player_entity_jump_height;
            entity->jumping = true;
        }
    } else {
        if (entity->vel.y < 0.0f && !zgl::KeyCheckDown(input_state, zgl::ek_key_code_space)) {
            entity->vel.y = 0.0f;
        }
    }

    TilemapProcessCollisions(tilemap, &entity->pos, &entity->vel, PlayerEntityGetColliderSize(entity->pos), k_player_entity_origin);

    entity->pos += entity->vel;
}

void PlayerProcessItemUsage(t_player_entity *const entity, const t_tilemap *const tilemap, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    if (entity->item_use_time > 0) {
        entity->item_use_time--;
    } else {
#if 0
        const t_inventory_slot hotbar_slot_selected = InventoryGet(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index);

        if (hotbar_slot_selected.quantity > 0) {
            const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

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

                    entity->item_use_time = g_item_types[hotbar_slot_selected.item_type_id].use_time;
                }
            }
#endif
    }
}

void PlayerEntityRender(const t_player_entity *const entity, const zgl::t_rendering_context rc, const t_assets *const assets) {
    SpriteRender(ek_sprite_id_player, rc, assets, {entity->pos.x, entity->pos.y}, k_player_entity_origin);
}
