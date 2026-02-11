#include "world_phase.h"

#include "camera.h"
#include "inventories.h"
#include "tiles.h"
#include "npcs.h"
#include "world_gen.h"
#include "pop_ups.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_gravity = 0.2f;

constexpr zcl::t_i32 k_player_respawn_duration = 120;
constexpr zcl::t_i32 k_player_invincible_duration = 30;
constexpr zcl::t_f32 k_player_move_spd = 1.5f;
constexpr zcl::t_f32 k_player_move_spd_acc = 0.2f;
constexpr zcl::t_f32 k_player_jump_height = 3.5f;
constexpr zcl::t_v2 k_player_origin = zcl::k_origin_center;
constexpr zcl::t_i32 k_player_flash_duration = 10;

constexpr zcl::t_v2_i k_tilemap_size = {8000, 400};

constexpr zcl::t_f32 k_ui_tile_highlight_alpha = 0.6f;
constexpr zcl::t_v2 k_ui_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_ui_player_health_bar_size = {240.0f, 24.0f};
constexpr zcl::t_f32 k_ui_player_health_bar_bg_alpha = 0.4f;
constexpr zcl::t_v2 k_ui_player_inventory_offs_top_left = {48.0f, 48.0f};
constexpr zcl::t_f32 k_ui_player_inventory_slot_size = 48.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_distance = 64.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_bg_alpha = 0.4f;

struct t_ui {
    zcl::t_i32 player_inventory_open;

    t_item_type_id cursor_held_item_type_id;
    zcl::t_i32 cursor_held_quantity;
};

struct t_player_meta {
    zcl::t_i32 respawn_time;

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

struct t_world_phase {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead? Maybe as a seed from the title screen?

    t_tilemap *tilemap;

    t_player_entity player_entity;
    t_player_meta player_meta;

    t_npc_manager npc_manager;

    t_pop_up_manager pop_up_manager;

    t_camera *camera;

    t_ui ui;
};

static zcl::t_v2 TilemapMoveContactByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    ZCL_ASSERT(jump_size > 0.0f);

    zcl::t_v2 pos_next = pos_current;

    const zcl::t_v2 jump_dir = zcl::k_cardinal_direction_normals[cardinal_dir_id];
    const zcl::t_v2 jump = jump_dir * jump_size;

    while (!TilemapCheckCollision(tilemap, ColliderCreate(pos_next + jump, collider_size, collider_origin))) {
        pos_next += jump;
    }

    return pos_next;
}

constexpr zcl::t_f32 k_tilemap_move_contact_jump_size_precise = 0.5f;

zcl::t_v2 TilemapMoveContact(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    zcl::t_v2 pos_next = pos_current;

    // Jump by tile intervals first, then make more precise contact.
    pos_next = TilemapMoveContactByJumpSize(pos_next, k_tile_size, cardinal_dir_id, collider_size, collider_origin, tilemap);
    pos_next = TilemapMoveContactByJumpSize(pos_next, k_tilemap_move_contact_jump_size_precise, cardinal_dir_id, collider_size, collider_origin, tilemap);

    return pos_next;
}

static void TilemapProcessCollisionsVertical(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_vertical = ColliderCreate({pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_vertical)) {
        *pos = TilemapMoveContactByJumpSize(*pos, k_tilemap_move_contact_jump_size_precise, *vel_y >= 0.0f ? zcl::ek_cardinal_direction_down : zcl::ek_cardinal_direction_up, collider_size, collider_origin, tilemap);
        *vel_y = 0.0f;
    }
}

void TilemapProcessCollisions(const t_tilemap *const tilemap, zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin) {
    const zcl::t_rect_f collider_hor = ColliderCreate({pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_hor)) {
        *pos = TilemapMoveContactByJumpSize(*pos, k_tilemap_move_contact_jump_size_precise, vel->x >= 0.0f ? zcl::ek_cardinal_direction_right : zcl::ek_cardinal_direction_left, collider_size, collider_origin, tilemap);
        vel->x = 0.0f;
    }

    TilemapProcessCollisionsVertical(pos, &vel->y, collider_size, collider_origin, tilemap);

    const zcl::t_rect_f collider_diagonal = ColliderCreate(*pos + *vel, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_diagonal)) {
        vel->x = 0.0f;
    }
}

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
        player_meta->inventory_hotbar_slot_selected_index += zcl::Round(scroll_offs.y);
        player_meta->inventory_hotbar_slot_selected_index = zcl::Wrap(player_meta->inventory_hotbar_slot_selected_index, 0, inventory_width);
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

t_world_phase *PhaseWorldInit(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena, zcl::t_arena *const temp_arena) {
    const auto result = zcl::ArenaPush<t_world_phase>(arena);

    result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    result->tilemap = GenWorld(k_tilemap_size, result->rng, arena, temp_arena);

    result->player_meta = CreatePlayerMeta(arena);

    result->player_entity = CreatePlayerEntity(&result->player_meta, result->tilemap);

    result->camera = CameraCreate(result->player_entity.pos, 2.0f, 0.3f, arena);

    SpawnNPC(&result->npc_manager, {k_tile_size * k_tilemap_size.x * 0.5f, 0.0f}, ek_npc_type_id_slime);

    return result;
}

t_world_phase_tick_result_id WorldPhaseTick(t_world_phase *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    t_world_phase_tick_result_id result_id = ek_world_phase_tick_result_id_normal;

    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    if (!world->player_entity.active) {
        ZCL_ASSERT(world->player_meta.respawn_time >= 0 && world->player_meta.respawn_time <= k_player_respawn_duration);

        if (world->player_meta.respawn_time > 0) {
            world->player_meta.respawn_time--;
        } else {
            world->player_entity = CreatePlayerEntity(&world->player_meta, world->tilemap);
        }
    }

    if (world->player_entity.active) {
        UpdatePlayerTimers(&world->player_entity);

        ProcessPlayerInventoryHotbarUpdates(&world->player_meta, input_state);

        PlayerUpdateMovement(&world->player_entity, world->tilemap, input_state);
    }

    ProcessNPCAIs(&world->npc_manager, world->tilemap);

    if (world->player_entity.active) {
        ProcessPlayerDeath(&world->player_meta, &world->player_entity);
    }

    ProcessNPCDeaths(&world->npc_manager);

    // @todo: Camera shake helpers.
    // @todo: Target position needs to be cached inside camera struct and updated via a distinct function.
    CameraMove(world->camera, world->player_entity.pos);

    UpdatePopUps(&world->pop_up_manager);

    return result_id;
}

#if 0
    static zcl::t_rect_i CalcCameraTilemapRect(const t_camera *const camera, const zcl::t_v2_i screen_size) {
        const zcl::t_f32 camera_scale = CameraGetScale(camera);

        const zcl::t_rect_f camera_rect = CameraCalcRect(camera, screen_size);

        const zcl::t_i32 camera_tilemap_left = static_cast<zcl::t_i32>(zcl::Floor(zcl::RectGetLeft(camera_rect) / k_tile_size));
        const zcl::t_i32 camera_tilemap_top = static_cast<zcl::t_i32>(zcl::Floor(zcl::RectGetTop(camera_rect) / k_tile_size));
        const zcl::t_i32 camera_tilemap_right = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetRight(camera_rect) / k_tile_size));
        const zcl::t_i32 camera_tilemap_bottom = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetBottom(camera_rect) / k_tile_size));

        return zcl::ClampWithinContainer(zcl::RectCreateI(camera_tilemap_left, camera_tilemap_top, camera_tilemap_right - camera_tilemap_left, camera_tilemap_bottom - camera_tilemap_top), zcl::RectCreateI({}, k_tilemap_size));
    }
#endif

void WorldPhaseRender(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, rc.screen_size);
    zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix, true, k_bg_color);

    // TilemapRender(world->tilemap, CalcCameraTilemapRect(world->camera, rc.screen_size), rc, assets);

    if (world->player_entity.active) {
        RenderPlayer(&world->player_entity, rc, assets);
    }

    RenderNPCs(&world->npc_manager, rc, assets);

    zgl::RendererPassEnd(rc);
}

void WorldPhaseRenderUI(const t_world_phase *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    RenderPopUps(rc, &world->pop_up_manager, world->camera, assets, temp_arena);
}

// Returns true iff a slot is hovered.
[[nodiscard]] static zcl::t_b8 GetPlayerInventoryHoveredSlotPos(const t_inventory *const inventory, const zcl::t_b8 inventory_open, const zcl::t_v2 cursor_position, zcl::t_v2_i *const o_slot_pos) {
    const zcl::t_v2 cursor_pos_rel_to_inventory_top_left = cursor_position - k_ui_player_inventory_offs_top_left;

    const zcl::t_v2_i inventory_size = InventoryGetSize(inventory);
    const zcl::t_v2 inventory_size_in_pixels = k_ui_player_inventory_slot_distance * zcl::V2IToF(inventory_size);

    if (cursor_pos_rel_to_inventory_top_left.x >= 0.0f && cursor_pos_rel_to_inventory_top_left.y >= 0.0f && cursor_pos_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_pos_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_pos = {
            static_cast<zcl::t_i32>(zcl::Floor(cursor_pos_rel_to_inventory_top_left.x / k_ui_player_inventory_slot_distance)),
            static_cast<zcl::t_i32>(zcl::Floor(cursor_pos_rel_to_inventory_top_left.y / k_ui_player_inventory_slot_distance)),
        };

        if (slot_pos.y == 0 || inventory_open) {
            const zcl::t_v2 cursor_pos_rel_to_slot_region = cursor_pos_rel_to_inventory_top_left - (zcl::V2IToF(slot_pos) * k_ui_player_inventory_slot_distance);

            if (cursor_pos_rel_to_slot_region.x < k_ui_player_inventory_slot_size && cursor_pos_rel_to_slot_region.y < k_ui_player_inventory_slot_size) {
                *o_slot_pos = slot_pos;
                return true;
            }
        }
    }

    return false;
}

static zcl::t_rect_f CalcPlayerInventorySlotRect(const zcl::t_v2_i slot_pos) {
    const zcl::t_v2 slot_pos_screen = k_ui_player_inventory_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_ui_player_inventory_slot_distance);
    const zcl::t_v2 slot_size = {k_ui_player_inventory_slot_size, k_ui_player_inventory_slot_size};

    return zcl::RectCreateF(slot_pos_screen, slot_size);
}

void ProcessPlayerInventoryInteraction(t_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        ui->player_inventory_open = !ui->player_inventory_open;
    }

    if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
        zcl::t_v2_i slot_hovered_pos;

        if (GetPlayerInventoryHoveredSlotPos(player_inventory, ui->player_inventory_open, cursor_pos, &slot_hovered_pos)) {
            const auto slot = InventoryGet(player_inventory, slot_hovered_pos);

            if (ui->cursor_held_quantity == 0) {
                ui->cursor_held_item_type_id = slot.item_type_id;
                ui->cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(player_inventory, slot_hovered_pos, slot.quantity);
            } else {
                if (slot.quantity == 0) {
                    InventoryAddAt(player_inventory, slot_hovered_pos, ui->cursor_held_item_type_id, ui->cursor_held_quantity);
                    ui->cursor_held_quantity = 0;
                }
            }
        }
    }
}

static zcl::t_str_mut DetermineCursorHoverStr(const zcl::t_v2 cursor_pos, const t_inventory *const player_inventory, const zcl::t_b8 player_inventory_open, const t_npc_manager *const npc_manager, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
    constexpr zcl::t_i32 k_str_len_limit = 32;

    {
        zcl::t_v2_i player_inventory_slot_hovered_pos;

        if (GetPlayerInventoryHoveredSlotPos(player_inventory, player_inventory_open, cursor_pos, &player_inventory_slot_hovered_pos)) {
            const t_inventory_slot player_inventory_slot_hovered = InventoryGet(player_inventory, player_inventory_slot_hovered_pos);

            if (player_inventory_slot_hovered.quantity > 0) {
                return zcl::StrClone(g_item_types[player_inventory_slot_hovered.item_type_id].name, arena);
            }
        }
    }

    for (zcl::t_i32 i = 0; i < k_npc_limit; i++) {
        if (!zcl::BitsetCheckSet(npc_manager->activity, i)) {
            continue;
        }

        const auto npc = &npc_manager->buf[i];
        const zcl::t_rect_f npc_collider = GetNPCCollider(npc->pos, npc->type_id);
        const zcl::t_rect_f npc_collider_screen = zcl::RectCreateF(CameraToScreenPos(zcl::RectGetPos(npc_collider), camera, screen_size), zcl::RectGetSize(npc_collider) * CameraGetScale(camera));

        if (zcl::CheckPointInRect(cursor_pos, npc_collider_screen)) {
            return zcl::StrClone(g_npc_types[npc->type_id].name, arena);
        }
    }

    return {};
}

static void UIRenderItem(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    SpriteRender(g_item_types[item_type_id].icon_sprite_id, rc, assets, pos, zcl::k_origin_center, 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rc, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *GetFont(assets, ek_font_id_eb_garamond_24), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

void RenderPlayerInventory(const zgl::t_rendering_context rc, const t_ui *const ui, const t_player_meta *const player_meta, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    const zcl::t_v2_i inventory_size = InventoryGetSize(player_meta->inventory);

    const zcl::t_i32 slot_cnt_y = ui->player_inventory_open ? inventory_size.y : 1;

    for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
        for (zcl::t_i32 slot_x = 0; slot_x < inventory_size.x; slot_x++) {
            const zcl::t_i32 slot_index = (slot_y * inventory_size.x) + slot_x;

            const auto slot = InventoryGet(player_meta->inventory, {slot_x, slot_y});

            const auto ui_slot_rect = CalcPlayerInventorySlotRect({slot_x, slot_y});

            const auto ui_slot_color = slot_y == 0 && player_meta->inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
            ZCL_ASSERT(ui_slot_color.a == 1.0f);

            zgl::RendererSubmitRect(rc, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_inventory_slot_bg_alpha));
            zgl::RendererSubmitRectOutlineOpaque(rc, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

            if (slot.quantity > 0) {
                UIRenderItem(slot.item_type_id, slot.quantity, rc, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
            }
        }
    }
}

void UIRenderPlayerHealth(const zgl::t_rendering_context rc, const zcl::t_i32 health, const zcl::t_i32 health_limit) {
    ZCL_ASSERT(health >= 0 && health <= health_limit);

    const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(rc.screen_size.x) - k_ui_player_health_bar_offs_top_right.x - k_ui_player_health_bar_size.x, k_ui_player_health_bar_offs_top_right.y};
    const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_ui_player_health_bar_size);

    zgl::RendererSubmitRect(rc, health_bar_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_health_bar_bg_alpha));

    zgl::RendererSubmitRectOutlineOpaque(rc, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);

    zgl::RendererSubmitRect(rc, zcl::RectCreateF(health_bar_rect.x, health_bar_rect.y, health_bar_rect.width * (static_cast<zcl::t_f32>(health) / health_limit), health_bar_rect.height), zcl::k_color_white);
}

void UIRenderPlayerDeathStr(const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 screen_center = zcl::V2IToF(rc.screen_size) / 2.0f;
    zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("You were slain..."), *GetFont(assets, ek_font_id_eb_garamond_80), screen_center, zcl::k_color_red, temp_arena, zcl::k_origin_center);
}

void UIRenderCursorHeldItem(const t_ui *const ui, const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    if (ui->cursor_held_quantity > 0) {
        UIRenderItem(ui->cursor_held_item_type_id, ui->cursor_held_quantity, rc, cursor_pos, assets, temp_arena);
    }
}

void UIRenderCursorHoverStr(const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_inventory *const player_inventory, const zcl::t_b8 player_inventory_open, const t_npc_manager *const npc_manager, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    const auto cursor_hover_str = DetermineCursorHoverStr(cursor_pos, player_inventory, player_inventory_open, npc_manager, camera, rc.screen_size, temp_arena);

    if (!zcl::StrCheckEmpty(cursor_hover_str)) {
        zgl::RendererSubmitStr(rc, cursor_hover_str, *GetFont(assets, ek_font_id_eb_garamond_32), cursor_pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}
