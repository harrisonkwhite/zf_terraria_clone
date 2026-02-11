#include "player.h"

#include "inventories.h"
#include "tiles.h"
#include "pop_ups.h"
#include "stray.h"

constexpr zcl::t_i32 k_player_invincible_duration = 30;
constexpr zcl::t_f32 k_player_move_spd = 1.5f;
constexpr zcl::t_f32 k_player_move_spd_acc = 0.2f;
constexpr zcl::t_f32 k_player_jump_height = 3.5f;
constexpr zcl::t_v2 k_player_origin = zcl::k_origin_center;
constexpr zcl::t_i32 k_player_flash_duration = 10;

struct t_player_meta {
    zcl::t_i32 health_limit;

    t_inventory *inventory;
    zcl::t_i32 inventory_hotbar_slot_selected_index;
};

struct t_player_entity {
    zcl::t_b8 active;

    zcl::t_i32 health;
    zcl::t_i32 invincible_time;

    zcl::t_i32 item_use_time;

    zcl::t_v2 pos;
    zcl::t_v2 vel;
    zcl::t_b8 jumping;

    zcl::t_i32 flash_time; // @note: Could be derived from invincible_time?
};

t_player_meta *PlayerCreateMeta(zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_player_meta>(arena);

    result->health_limit = 100;

    result->inventory = InventoryCreate({7, 4}, arena);
    InventoryAdd(result->inventory, ek_item_type_id_copper_pickaxe, 1);

    return result;
}

static zcl::t_v2 PlayerGetColliderSize() {
    return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect));
}

t_player_entity *PlayerCreateEntity(const t_player_meta *const player_meta, const t_tilemap *const tilemap, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_player_entity>(arena);

    result->active = true;

    result->health = player_meta->health_limit;

    const zcl::t_v2 collider_size = PlayerGetColliderSize();
    const zcl::t_f32 x = (TilemapGetSize(tilemap).x * k_tile_size) / 2.0f;
    result->pos = MakeContactWithTilemap({x, -collider_size.y * (1.0f - k_player_origin.y)}, zcl::ek_cardinal_direction_down, collider_size, k_player_origin, tilemap);

    return result;
}

void PlayerUpdateTimers(t_player_entity *const player_entity) {
    ZCL_ASSERT(player_entity->active);

    if (player_entity->invincible_time > 0) {
        player_entity->invincible_time--;
    }

    if (player_entity->flash_time > 0) {
        player_entity->flash_time--;
    }
}

static zcl::t_b8 PlayerCheckGrounded(const zcl::t_v2 player_entity_pos, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_below = zcl::RectCreateTranslated(PlayerGetCollider(player_entity_pos), {0.0f, 1.0f});
    return TilemapCheckCollision(tilemap, collider_below);
}

void PlayerUpdateMovement(t_player_entity *const player_entity, const zgl::t_input_state *const input_state, const zcl::t_f32 gravity, const t_tilemap *const tilemap) {
    ZCL_ASSERT(player_entity->active);

    const zcl::t_f32 move_axis = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d) - zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);

    const zcl::t_f32 move_spd_targ = move_axis * k_player_move_spd;

    if (player_entity->vel.x < move_spd_targ) {
        player_entity->vel.x += zcl::CalcMin(move_spd_targ - player_entity->vel.x, k_player_move_spd_acc);
    } else if (player_entity->vel.x > move_spd_targ) {
        player_entity->vel.x -= zcl::CalcMin(player_entity->vel.x - move_spd_targ, k_player_move_spd_acc);
    }

    player_entity->vel.y += gravity;

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

    ProcessTilemapCollisions(&player_entity->pos, &player_entity->vel, PlayerGetColliderSize(), k_player_origin, tilemap);

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
        player_meta->inventory_hotbar_slot_selected_index += zcl::Round(scroll_offs.y);
        player_meta->inventory_hotbar_slot_selected_index = zcl::Wrap(player_meta->inventory_hotbar_slot_selected_index, 0, inventory_width);
    }
}

void PlayerProcessDeath(t_player_entity *const player_entity) {
    ZCL_ASSERT(player_entity->active);

    if (player_entity->health == 0) {
        player_entity->active = false;
    }
}

void PlayerProcessItemUsage(t_player_entity *const player_entity, const zgl::t_input_state *const input_state, t_player_meta *const player_meta, t_camera *const camera, t_tilemap *const tilemap, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(player_entity->active);

    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    if (player_entity->item_use_time > 0) {
        player_entity->item_use_time--;
    } else {
        const t_inventory_slot hotbar_slot_selected = InventoryGet(player_meta->inventory, {player_meta->inventory_hotbar_slot_selected_index, 0});

        if (hotbar_slot_selected.quantity > 0) {
            const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

            const zcl::t_b8 item_use = g_item_types[item_type_id].use_hold ? zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left) : zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left);

            if (item_use) {
                const t_item_type_use_func_context item_use_func_context = {
                    .temp_arena = temp_arena,
                    .cursor_pos = cursor_pos,
                    .screen_size = screen_size,
                    .camera = camera,
                    .tilemap = tilemap,
                    .player_meta = player_meta,
                    .player_entity = player_entity,
                };

                const zcl::t_b8 item_use_success = g_item_type_use_funcs[item_type_id](item_use_func_context);

                if (item_use_success) {
                    if (g_item_types[item_type_id].use_consume) {
                        InventoryRemoveAt(player_meta->inventory, {player_meta->inventory_hotbar_slot_selected_index, 0}, 1);
                    }

                    player_entity->item_use_time = g_item_types[hotbar_slot_selected.item_type_id].use_time;
                }
            }
        }
    }
}

void PlayerHurt(t_player_entity *const player_entity, const zcl::t_i32 damage, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng) {
    ZCL_ASSERT(player_entity->active);
    ZCL_ASSERT(damage > 0);

    if (player_entity->invincible_time > 0) {
        return;
    }

    player_entity->health -= zcl::CalcMin(player_entity->health, damage);
    player_entity->invincible_time = k_player_invincible_duration;
    player_entity->flash_time = k_player_flash_duration;

    PopUpSpawnDamage(pop_up_manager, player_entity->pos, damage, rng);
}

void PlayerRender(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets) {
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

zcl::t_b8 PlayerCheckAlive(const t_player_entity *const player_entity) {
    return player_entity->active;
}

zcl::t_i32 PlayerGetHealth(const t_player_entity *const player_entity) {
    return player_entity->health;
}

zcl::t_i32 PlayerGetHealthLimit(const t_player_meta *const player_meta) {
    return player_meta->health_limit;
}

t_inventory *PlayerGetInventory(const t_player_meta *const player_meta) {
    return player_meta->inventory;
}

zcl::t_i32 PlayerGetInventoryHotbarSlotSelectedIndex(const t_player_meta *const player_meta) {
    return player_meta->inventory_hotbar_slot_selected_index;
}

zcl::t_v2 PlayerGetPosition(const t_player_entity *const player_entity) {
    return player_entity->pos;
}

zcl::t_rect_f PlayerGetCollider(const zcl::t_v2 pos) {
    return ColliderCreate(pos, PlayerGetColliderSize(), k_player_origin);
}
