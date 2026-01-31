#include "world.h"

#include "camera.h"
#include "inventory.h"
#include "tiles.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_gravity = 0.2f;

constexpr zcl::t_f32 k_player_entity_move_spd = 1.5f;
constexpr zcl::t_f32 k_player_entity_move_spd_acc = 0.2f;
constexpr zcl::t_f32 k_player_entity_jump_height = 3.5f;
constexpr zcl::t_v2 k_player_entity_origin = zcl::k_origin_center;

constexpr zcl::t_i32 k_player_inventory_slot_cnt = 28;

constexpr zcl::t_v2 k_ui_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_ui_player_health_bar_size = {240.0f, 24.0f};

constexpr zcl::t_v2 k_ui_player_inventory_offs_top_left = {48.0f, 48.0f};

constexpr zcl::t_i32 k_ui_player_inventory_slot_cnt_x = 7;
static_assert(k_ui_player_inventory_slot_cnt_x <= 9, "You need to be able to map numeric keys to hotbar slots.");

constexpr zcl::t_i32 k_ui_player_inventory_slot_cnt_y = 4;

static_assert(k_ui_player_inventory_slot_cnt_x * k_ui_player_inventory_slot_cnt_y == k_player_inventory_slot_cnt);

constexpr zcl::t_f32 k_ui_player_inventory_slot_size = 48.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_distance = 64.0f;
constexpr zcl::t_f32 k_ui_player_inventory_slot_bg_alpha = 0.2f;

struct t_player_entity {
    zcl::t_v2 pos;
    zcl::t_v2 vel;
    zcl::t_b8 jumping;
};

struct t_world {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?

    zgl::t_gfx_resource_group *gfx_resource_group;
    zgl::t_gfx_resource *texture_target;

    zcl::t_i32 player_health;
    zcl::t_i32 player_health_limit;

    t_inventory *player_inventory;

    t_tilemap *tilemap;

    t_player_entity player_entity;

    t_camera *camera;

    struct {
        zcl::t_i32 player_inventory_open;
        zcl::t_i32 player_inventory_hotbar_slot_selected_index;

        t_item_type_id cursor_held_item_type_id;
        zcl::t_i32 cursor_held_quantity;
    } ui;
};

static zcl::t_v2 PlayerEntityColliderGetSize(const zcl::t_v2 pos) {
    return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect));
}

static zcl::t_rect_f PlayerEntityColliderCreate(const zcl::t_v2 pos) {
    return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect)), k_player_entity_origin);
}

static zcl::t_b8 PlayerEntityCheckGrounded(const zcl::t_v2 entity_pos, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_below = zcl::RectCreateTranslated(PlayerEntityColliderCreate(entity_pos), {0.0f, 1.0f});
    return TilemapCheckCollision(tilemap, collider_below);
}


// ============================================================
// @section: Collision
// ============================================================

static zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    ZCL_ASSERT(jump_size > 0.0f);

    zcl::t_v2 pos_next = pos_current;

    const zcl::t_v2 jump_dir = zcl::k_cardinal_direction_normals[cardinal_dir_id];
    const zcl::t_v2 jump = jump_dir * jump_size;

    while (!TilemapCheckCollision(tilemap, ColliderCreate(pos_next + jump, collider_size, collider_origin))) {
        pos_next += jump;
    }

    return pos_next;
}

constexpr zcl::t_f32 k_tilemap_contact_jump_size_precise = 0.5f;

static zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    zcl::t_v2 pos_next = pos_current;

    // Jump by tile intervals first, then make more precise contact.
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_tile_size, cardinal_dir_id, collider_size, collider_origin, tilemap);
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_tilemap_contact_jump_size_precise, cardinal_dir_id, collider_size, collider_origin, tilemap);

    return pos_next;
}

static void ProcessTileCollisionsVertical(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_vertical = ColliderCreate({pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_vertical)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_jump_size_precise, *vel_y >= 0.0f ? zcl::ek_cardinal_direction_down : zcl::ek_cardinal_direction_up, collider_size, collider_origin, tilemap);
        *vel_y = 0.0f;
    }
}

static void ProcessTilemapCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_hor = ColliderCreate({pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_hor)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_jump_size_precise, vel->x >= 0.0f ? zcl::ek_cardinal_direction_right : zcl::ek_cardinal_direction_left, collider_size, collider_origin, tilemap);
        vel->x = 0.0f;
    }

    ProcessTileCollisionsVertical(pos, &vel->y, collider_size, collider_origin, tilemap);

    const zcl::t_rect_f collider_diagonal = ColliderCreate(*pos + *vel, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_diagonal)) {
        vel->x = 0.0f;
    }
}

// ============================================================


static void PlayerEntityProcessMovement(t_player_entity *const player, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state) {
    const zcl::t_f32 move_axis = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d) - zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);

    const zcl::t_f32 move_spd_targ = move_axis * k_player_entity_move_spd;

    if (player->vel.x < move_spd_targ) {
        player->vel.x += zcl::CalcMin(move_spd_targ - player->vel.x, k_player_entity_move_spd_acc);
    } else if (player->vel.x > move_spd_targ) {
        player->vel.x -= zcl::CalcMin(player->vel.x - move_spd_targ, k_player_entity_move_spd_acc);
    }

    player->vel.y += k_gravity;

    const zcl::t_b8 grounded = PlayerEntityCheckGrounded(player->pos, tilemap);

    if (grounded) {
        player->jumping = false;
    }

    if (!player->jumping) {
        if (grounded && zgl::KeyCheckPressed(input_state, zgl::ek_key_code_space)) {
            player->vel.y = -k_player_entity_jump_height;
            player->jumping = true;
        }
    } else {
        if (player->vel.y < 0.0f && !zgl::KeyCheckDown(input_state, zgl::ek_key_code_space)) {
            player->vel.y = 0.0f;
        }
    }

    ProcessTilemapCollisions(&player->pos, &player->vel, PlayerEntityColliderGetSize(player->pos), k_player_entity_origin, tilemap);

    player->pos += player->vel;
}

static void PlayerEntityRender(const t_player_entity *const player, const zgl::t_rendering_context rendering_context, const t_assets *const assets) {
    SpriteRender(ek_sprite_id_player, rendering_context, assets, player->pos, k_player_entity_origin);
}

static t_tilemap *WorldGen(zcl::t_rng *const rng, zcl::t_arena *const arena) {
    const auto tilemap = TilemapCreate(arena);

    zcl::t_static_array<zcl::t_i32, k_tilemap_size.x> ground_offsets;

    constexpr zcl::t_i32 k_ground_height = 10;

    zcl::t_i32 ground_offs_pen = zcl::RandGenI32InRange(rng, 0, k_ground_height);

    for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
        ground_offsets[x] = ground_offs_pen;

        if (zcl::RandGenPerc(rng) < 0.3f) {
            const zcl::t_b8 down = (ground_offs_pen == 0 || zcl::RandGenPerc(rng) < 0.5f) && ground_offs_pen < k_ground_height - 1;
            ground_offs_pen += down ? 1 : -1;
        }
    }

    const zcl::t_i32 ground_tilemap_y_begin = k_tilemap_size.y / 3.0f;

    for (zcl::t_i32 gy = 0; gy < k_ground_height; gy++) {
        for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
            if (gy >= ground_offsets[x]) {
                TilemapAdd(tilemap, {x, ground_tilemap_y_begin + gy}, ek_tile_type_id_dirt);
            }
        }
    }

    for (zcl::t_i32 y = ground_tilemap_y_begin + k_ground_height; y < k_tilemap_size.y; y++) {
        for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
            TilemapAdd(tilemap, {x, y}, ek_tile_type_id_dirt);
        }
    }

    return tilemap;
}

t_world *WorldCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_world>(arena);

    result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    result->gfx_resource_group = zgl::GFXResourceGroupCreate(gfx_ticket, arena);

    result->texture_target = zgl::TextureCreateTarget(gfx_ticket, zgl::BackbufferGetSize(gfx_ticket) / 2, result->gfx_resource_group);

    result->player_health_limit = 100;
    result->player_health = result->player_health_limit;

    result->tilemap = WorldGen(result->rng, arena);

    const zcl::t_v2 world_size = zcl::V2IToF(k_tilemap_size * k_tile_size);
    result->player_entity.pos = MakeContactWithTilemap({world_size.x * 0.5f, 0.0f}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(PlayerEntityColliderCreate(result->player_entity.pos)), k_player_entity_origin, result->tilemap);

    result->camera = CameraCreate(result->player_entity.pos, 0.3f, arena);

    return result;
}

void WorldDestroy(t_world *const world, const zgl::t_gfx_ticket_mut gfx_ticket) {
    zgl::GFXResourceGroupDestroy(gfx_ticket, world->gfx_resource_group);
    zcl::ZeroClearItem(world);
}

t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i window_framebuffer_size, zcl::t_arena *const temp_arena) {
    t_world_tick_result_id result_id = ek_world_tick_result_id_normal;

    PlayerEntityProcessMovement(&world->player_entity, world->tilemap, input_state);
    CameraMove(world->camera, world->player_entity.pos);

    return result_id;
}

void WorldRender(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const zcl::t_v2_i backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, backbuffer_size / 2);
    zgl::RendererPassBeginOffscreen(rendering_context, world->texture_target, camera_view_matrix, true, k_bg_color);

#if 0
    TilemapRender(world->tilemap, CameraCalcRectTilemap(world->camera, backbuffer_size), rendering_context, assets);
#endif

    PlayerEntityRender(&world->player_entity, rendering_context, assets);

    zgl::RendererPassEnd(rendering_context);

    zgl::RendererPassBegin(rendering_context, backbuffer_size);

    zgl::RendererSubmitTexture(rendering_context, world->texture_target, {}, {}, zcl::k_origin_top_left, 0.0f, {2.0f, 2.0f});

    zgl::RendererPassEnd(rendering_context);
}

#if 0

constexpr zcl::t_i32 k_player_inventory_slot_cnt = 28;

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_gravity = 0.2f;

constexpr zcl::t_v2 k_player_inventory_offs_top_left = {48.0f, 48.0f};
constexpr zcl::t_f32 k_player_inventory_slot_size = 48.0f;
constexpr zcl::t_f32 k_player_inventory_slot_distance = 64.0f;
constexpr zcl::t_f32 k_player_inventory_slot_bg_alpha = 0.2f;

constexpr zcl::t_v2 k_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_player_health_bar_size = {240.0f, 24.0f};

constexpr zcl::t_i32 k_player_inventory_slot_cnt_x = 7;
static_assert(k_player_inventory_slot_cnt_x <= 9, "You need to be able to map numeric keys to hotbar slots.");

constexpr zcl::t_i32 k_player_inventory_slot_cnt_y = 4;

static_assert(k_player_inventory_slot_cnt_x * k_player_inventory_slot_cnt_y == k_player_inventory_slot_cnt);

t_tilemap *WorldGen(zcl::t_rng *const rng, zcl::t_arena *const arena) {
    const auto tilemap = TilemapCreate(arena);

    zcl::t_static_array<zcl::t_i32, k_tilemap_size.x> ground_offsets;

    constexpr zcl::t_i32 k_ground_height = 10;

    zcl::t_i32 ground_offs_pen = zcl::RandGenI32InRange(rng, 0, k_ground_height);

    for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
        ground_offsets[x] = ground_offs_pen;

        if (zcl::RandGenPerc(rng) < 0.3f) {
            const zcl::t_b8 down = (ground_offs_pen == 0 || zcl::RandGenPerc(rng) < 0.5f) && ground_offs_pen < k_ground_height - 1;
            ground_offs_pen += down ? 1 : -1;
        }
    }

    const zcl::t_i32 ground_tilemap_y_begin = k_tilemap_size.y / 3.0f;

    for (zcl::t_i32 gy = 0; gy < k_ground_height; gy++) {
        for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
            if (gy >= ground_offsets[x]) {
                TilemapAdd(tilemap, {x, ground_tilemap_y_begin + gy}, ek_tile_type_id_dirt);
            }
        }
    }

    for (zcl::t_i32 y = ground_tilemap_y_begin + k_ground_height; y < k_tilemap_size.y; y++) {
        for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
            TilemapAdd(tilemap, {x, y}, ek_tile_type_id_dirt);
        }
    }

    return tilemap;
}

struct t_world_ui {
    zcl::t_i32 inventory_open;
    zcl::t_i32 inventory_hotbar_slot_selected_index;

    t_item_type_id cursor_held_item_type_id;
    zcl::t_i32 cursor_held_quantity;
};

static void RenderItem(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rendering_context, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    SpriteRender(g_item_types[item_type_id].icon_sprite_id, rendering_context, assets, pos, zcl::k_origin_center, 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rendering_context, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *GetFont(assets, ek_font_id_eb_garamond_24), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

// Returns -1 if no slot is hovered.
static zcl::t_i32 InventoryGetHoveredSlotIndex(const zcl::t_v2 cursor_position, const zcl::t_b8 inventory_open) {
    const zcl::t_v2 cursor_position_rel_to_inventory_top_left = cursor_position - k_player_inventory_offs_top_left;
    const zcl::t_v2 inventory_size_in_pixels = k_player_inventory_slot_distance * zcl::t_v2{k_player_inventory_slot_cnt_x, k_player_inventory_slot_cnt_y};

    if (cursor_position_rel_to_inventory_top_left.x >= 0.0f && cursor_position_rel_to_inventory_top_left.y >= 0.0f && cursor_position_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_position_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_position_in_grid = {
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.x / k_player_inventory_slot_distance)),
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.y / k_player_inventory_slot_distance)),
        };

        if (slot_position_in_grid.y == 0 || inventory_open) {
            const zcl::t_v2 cursor_position_rel_to_slot_region = cursor_position_rel_to_inventory_top_left - (zcl::V2IToF(slot_position_in_grid) * k_player_inventory_slot_distance);

            if (cursor_position_rel_to_slot_region.x < k_player_inventory_slot_size && cursor_position_rel_to_slot_region.y < k_player_inventory_slot_size) {
                return (slot_position_in_grid.y * k_player_inventory_slot_cnt_x) + slot_position_in_grid.x;
            }
        }
    }

    return -1;
}

static zcl::t_rect_f InventoryCalcSlotRect(const zcl::t_i32 slot_index) {
    ZCL_ASSERT(slot_index >= 0 && slot_index < k_player_inventory_slot_cnt);

    const zcl::t_v2_i slot_pos = {slot_index % k_player_inventory_slot_cnt_x, slot_index / k_player_inventory_slot_cnt_x};
    const zcl::t_v2 ui_slot_pos = k_player_inventory_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_player_inventory_slot_distance);
    const zcl::t_v2 ui_slot_size = {k_player_inventory_slot_size, k_player_inventory_slot_size};

    return zcl::RectCreateF(ui_slot_pos, ui_slot_size);
}

t_world_ui *WorldUICreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_world_ui>(arena);
}

void WorldUITick(t_world_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state) {
    // @todo: Scroll support.

    for (zcl::t_i32 i = 0; i < k_player_inventory_slot_cnt_x; i++) {
        if (zgl::KeyCheckPressed(input_state, static_cast<zgl::t_key_code>(zgl::ek_key_code_1 + i))) {
            ui->inventory_hotbar_slot_selected_index = i;
            break;
        }
    }

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        ui->inventory_open = !ui->inventory_open;
    }

    if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
        const zcl::t_i32 slot_hovered_index = InventoryGetHoveredSlotIndex(zgl::CursorGetPos(input_state), ui->inventory_open);

        if (slot_hovered_index != -1) {
            const auto slot = InventoryGet(player_inventory, slot_hovered_index);

            if (ui->cursor_held_quantity == 0) {
                ui->cursor_held_item_type_id = slot.item_type_id;
                ui->cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(player_inventory, slot_hovered_index, slot.quantity);
            } else {
                if (slot.quantity == 0) {
                    InventoryAddAt(player_inventory, slot_hovered_index, ui->cursor_held_item_type_id, ui->cursor_held_quantity);
                    ui->cursor_held_quantity = 0;
                }
            }
        }
    }
}

void WorldUIRender(const t_world_ui *const ui, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const auto backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

    #if 0
    //
    // Inventory
    //
    const zcl::t_i32 slot_cnt_y = ui->inventory_open ? k_player_inventory_slot_cnt_y : 1;

    for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
        for (zcl::t_i32 slot_x = 0; slot_x < k_player_inventory_slot_cnt_x; slot_x++) {
            const zcl::t_i32 slot_index = (slot_y * k_player_inventory_slot_cnt_x) + slot_x;

            const auto slot = InventoryGet(ui->inventory, slot_index);

            const auto ui_slot_rect = PlayerInventoryUISlotCalcRect(slot_index);

            const auto ui_slot_color = slot_y == 0 && ui->inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
            ZCL_ASSERT(ui_slot_color.a == 1.0f);

            zgl::RendererSubmitRect(rendering_context, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_player_inventory_slot_bg_alpha));
            zgl::RendererSubmitRectOutlineOpaque(rendering_context, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

            if (slot.quantity > 0) {
                RenderItemUI(slot.item_type_id, slot.quantity, rendering_context, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
            }
        }
    }

    //
    // Health
    //
    const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(backbuffer_size.x) - k_player_health_bar_offs_top_right.x - k_player_health_bar_size.x, k_player_health_bar_offs_top_right.y};
    const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_player_health_bar_size);

    zgl::RendererSubmitRectOutlineOpaque(rendering_context, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);

    //
    // Cursor Held
    //
    if (ui->cursor_held_quantity > 0) {
        RenderItemUI(ui->cursor_held_item_type_id, ui->cursor_held_quantity, rendering_context, zgl::CursorGetPos(input_state), assets, temp_arena);
    }
    #endif
}


// ============================================================
// @section: Player
// ============================================================

constexpr zcl::t_v2 k_player_origin = zcl::k_origin_center;

constexpr zcl::t_f32 k_player_move_spd = 1.5f;
constexpr zcl::t_f32 k_player_move_spd_acc = 0.2f;

constexpr zcl::t_f32 k_player_jump_height = 3.5f;

struct t_player {
    zcl::t_v2 position;
    zcl::t_v2 velocity;
    zcl::t_b8 jumping;
};

static zcl::t_v2 PlayerColliderGetSize(const zcl::t_v2 pos) {
    return zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect));
}

static zcl::t_rect_f PlayerColliderCreate(const zcl::t_v2 pos) {
    return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[ek_sprite_id_player].src_rect)), k_player_origin);
}

static zcl::t_b8 PlayerCheckGrounded(const zcl::t_v2 player_pos, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_below = zcl::RectCreateTranslated(PlayerColliderCreate(player_pos), {0.0f, 1.0f});
    return TilemapCheckCollision(tilemap, collider_below);
}

zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    ZCL_ASSERT(jump_size > 0.0f);

    zcl::t_v2 pos_next = pos_current;

    const zcl::t_v2 jump_dir = zcl::k_cardinal_direction_normals[cardinal_dir_id];
    const zcl::t_v2 jump = jump_dir * jump_size;

    while (!TilemapCheckCollision(tilemap, ColliderCreate(pos_next + jump, collider_size, collider_origin))) {
        pos_next += jump;
    }

    return pos_next;
}

constexpr zcl::t_f32 k_tilemap_contact_jump_size_precise = 0.5f;

zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    zcl::t_v2 pos_next = pos_current;

    // Jump by tile intervals first, then make more precise contact.
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_tile_size, cardinal_dir_id, collider_size, collider_origin, tilemap);
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_tilemap_contact_jump_size_precise, cardinal_dir_id, collider_size, collider_origin, tilemap);

    return pos_next;
}

static void ProcessTileCollisionsVertical(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_vertical = ColliderCreate({pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_vertical)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_jump_size_precise, *vel_y >= 0.0f ? zcl::ek_cardinal_direction_down : zcl::ek_cardinal_direction_up, collider_size, collider_origin, tilemap);
        *vel_y = 0.0f;
    }
}

void ProcessTilemapCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_hor = ColliderCreate({pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_hor)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_jump_size_precise, vel->x >= 0.0f ? zcl::ek_cardinal_direction_right : zcl::ek_cardinal_direction_left, collider_size, collider_origin, tilemap);
        vel->x = 0.0f;
    }

    ProcessTileCollisionsVertical(pos, &vel->y, collider_size, collider_origin, tilemap);

    const zcl::t_rect_f collider_diagonal = ColliderCreate(*pos + *vel, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_diagonal)) {
        vel->x = 0.0f;
    }
}

static void PlayerProcessMovement(t_player *const player, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state) {
    const zcl::t_f32 move_axis = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d) - zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);

    const zcl::t_f32 move_spd_targ = move_axis * k_player_move_spd;

    if (player->velocity.x < move_spd_targ) {
        player->velocity.x += zcl::CalcMin(move_spd_targ - player->velocity.x, k_player_move_spd_acc);
    } else if (player->velocity.x > move_spd_targ) {
        player->velocity.x -= zcl::CalcMin(player->velocity.x - move_spd_targ, k_player_move_spd_acc);
    }

    player->velocity.y += k_gravity;

    const zcl::t_b8 grounded = PlayerCheckGrounded(player->position, tilemap);

    if (grounded) {
        player->jumping = false;
    }

    if (!player->jumping) {
        if (grounded && zgl::KeyCheckPressed(input_state, zgl::ek_key_code_space)) {
            player->velocity.y = -k_player_jump_height;
            player->jumping = true;
        }
    } else {
        if (player->velocity.y < 0.0f && !zgl::KeyCheckDown(input_state, zgl::ek_key_code_space)) {
            player->velocity.y = 0.0f;
        }
    }

    ProcessTilemapCollisions(&player->position, &player->velocity, PlayerColliderGetSize(player->position), k_player_origin, tilemap);

    player->position += player->velocity;
}

static void PlayerRender(const t_player *const player, const zgl::t_rendering_context rendering_context, const t_assets *const assets) {
    SpriteRender(ek_sprite_id_player, rendering_context, assets, player->position, k_player_origin);
}

// ============================================================


struct t_world {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?

    t_world_ui *ui;

    t_camera *camera;
    t_tilemap *tilemap;
    t_player player;

    zcl::t_b8 dead;

    t_inventory *inventory;

    zcl::t_i32 health;
    zcl::t_i32 health_limit;

    zgl::t_gfx_resource_group *gfx_resource_group;
    zgl::t_gfx_resource *texture_target;
};

t_world *WorldCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_world>(arena);

    result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    result->inventory = InventoryCreate(k_player_inventory_slot_cnt, arena);
    InventoryAdd(result->inventory, ek_item_type_id_dirt_block, 3);
    InventoryAdd(result->inventory, ek_item_type_id_stone_block, 7);

    result->tilemap = WorldGen(result->rng, arena);

    const zcl::t_v2 world_size = zcl::V2IToF(k_tilemap_size * k_tile_size);
    result->player.position = MakeContactWithTilemap({world_size.x * 0.5f, 0.0f}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(PlayerColliderCreate(result->player.position)), k_player_origin, result->tilemap);

    result->camera = CameraCreate(result->player.position, 0.3f, arena);

    result->gfx_resource_group = zgl::GFXResourceGroupCreate(gfx_ticket, arena);

    result->texture_target = zgl::TextureCreateTarget(gfx_ticket, zgl::BackbufferGetSize(gfx_ticket) / 2, result->gfx_resource_group);

    result->ui = WorldUICreate(arena);

    return result;
}

void WorldDestroy(t_world *const world, const zgl::t_gfx_ticket_mut gfx_ticket) {
    zgl::GFXResourceGroupDestroy(gfx_ticket, world->gfx_resource_group);
    zcl::ZeroClearItem(world);
}

t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i window_framebuffer_size, zcl::t_arena *const temp_arena) {
    t_world_tick_result_id result_id = ek_world_tick_result_id_normal;

    WorldUITick(world->ui, world->inventory, input_state);

    PlayerProcessMovement(&world->player, world->tilemap, input_state);
    CameraMove(world->camera, world->player.position);

    return result_id;
}

void WorldRender(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const zcl::t_v2_i backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, backbuffer_size / 2);
    zgl::RendererPassBeginOffscreen(rendering_context, world->texture_target, camera_view_matrix, true, k_bg_color);

    #if 0
    TilemapRender(world->tilemap, CameraCalcRectTilemap(world->camera, backbuffer_size), rendering_context, assets);
    #endif

    PlayerRender(&world->player, rendering_context, assets);

    zgl::RendererPassEnd(rendering_context);

    zgl::RendererPassBegin(rendering_context, backbuffer_size);

    zgl::RendererSubmitTexture(rendering_context, world->texture_target, {}, {}, zcl::k_origin_top_left, 0.0f, {2.0f, 2.0f});

    zgl::RendererPassEnd(rendering_context);
}
#endif
