// @todo: A cleanup of this stinky file.
// @todo: World UI logic should be in its own file. Both render and tick.

#include "world_private.h"

#include "sprites.h"
#include "inventory.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_gravity = 0.2f;


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
    return TileCollisionCheck(tilemap, collider_below);
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

    ProcessTileCollisions(&player->position, &player->velocity, PlayerColliderGetSize(player->position), k_player_origin, tilemap);

    player->position += player->velocity;
}

static void PlayerRender(const t_player *const player, const zgl::t_rendering_context rendering_context, const t_assets *const assets) {
    SpriteRender(ek_sprite_id_player, rendering_context, assets, player->position, k_player_origin);
}

// ============================================================


// ============================================================
// @section: Player Metadata
// ============================================================

constexpr zcl::t_i32 k_player_inventory_slot_cnt_x = 7;
static_assert(k_player_inventory_slot_cnt_x <= 9, "You need to be able to map numeric keys to hotbar slots.");

constexpr zcl::t_i32 k_player_inventory_slot_cnt_y = 4;

constexpr zcl::t_i32 k_player_inventory_slot_cnt = k_player_inventory_slot_cnt_x * k_player_inventory_slot_cnt_y;

constexpr zcl::t_v2 k_player_inventory_ui_offs_top_left = {48.0f, 48.0f};
constexpr zcl::t_f32 k_player_inventory_ui_slot_size = 48.0f;
constexpr zcl::t_f32 k_player_inventory_ui_slot_distance = 64.0f;
constexpr zcl::t_f32 k_player_inventory_ui_slot_bg_alpha = 0.2f;

constexpr zcl::t_v2 k_player_health_bar_offs_top_right = {48.0f, 48.0f};
constexpr zcl::t_v2 k_player_health_bar_size = {240.0f, 24.0f};

// @todo: This is a very bizarre grouping. Just drop it and put all in world.
struct t_player_meta {
    zcl::t_b8 dead;

    t_inventory *inventory;
    zcl::t_i32 inventory_open;
    zcl::t_i32 inventory_hotbar_slot_selected_index;

    t_item_type_id cursor_held_item_type_id;
    zcl::t_i32 cursor_held_quantity;

    zcl::t_i32 health;
    zcl::t_i32 health_limit;
};

static zcl::t_rect_f PlayerInventoryUISlotCalcRect(const zcl::t_i32 slot_index) {
    ZCL_ASSERT(slot_index >= 0 && slot_index < k_player_inventory_slot_cnt);

    const zcl::t_v2_i slot_pos = {slot_index % k_player_inventory_slot_cnt_x, slot_index / k_player_inventory_slot_cnt_x};
    const zcl::t_v2 ui_slot_pos = k_player_inventory_ui_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_player_inventory_ui_slot_distance);
    const zcl::t_v2 ui_slot_size = {k_player_inventory_ui_slot_size, k_player_inventory_ui_slot_size};

    return zcl::RectCreateF(ui_slot_pos, ui_slot_size);
}

// ============================================================


struct t_world {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?
    t_player_meta player_meta;
    t_camera camera;
    t_tilemap tilemap;
    t_player player;

    zgl::t_gfx_resource_group *gfx_resource_group;
    zgl::t_gfx_resource *texture_target;
};

t_world *WorldCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_world>(arena);

    result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    result->player_meta.inventory = InventoryCreate(k_player_inventory_slot_cnt, arena);
    InventoryAdd(result->player_meta.inventory, ek_item_type_id_dirt_block, 3);
    InventoryAdd(result->player_meta.inventory, ek_item_type_id_stone_block, 7);

    WorldGen(result->rng, &result->tilemap);

    const zcl::t_v2 world_size = zcl::V2IToF(k_tilemap_size * k_tile_size);
    result->player.position = MakeContactWithTilemap({world_size.x * 0.5f, 0.0f}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(PlayerColliderCreate(result->player.position)), k_player_origin, &result->tilemap);

    result->camera.position = result->player.position;

    result->gfx_resource_group = zgl::GFXResourceGroupCreate(gfx_ticket, arena);

    result->texture_target = zgl::TextureCreateTarget(gfx_ticket, zgl::BackbufferGetSize(gfx_ticket) / 2, result->gfx_resource_group);

    return result;
}

void WorldDestroy(t_world *const world, const zgl::t_gfx_ticket_mut gfx_ticket) {
    zgl::GFXResourceGroupDestroy(gfx_ticket, world->gfx_resource_group);
    zcl::ZeroClearItem(world);
}

// Returns -1 if no slot is hovered.
static zcl::t_i32 PlayerInventoryUIGetHoveredSlotIndex(const zcl::t_v2 cursor_position, const zcl::t_b8 inventory_open) {
    const zcl::t_v2 cursor_position_rel_to_inventory_top_left = cursor_position - k_player_inventory_ui_offs_top_left;
    const zcl::t_v2 inventory_size_in_pixels = k_player_inventory_ui_slot_distance * zcl::t_v2{k_player_inventory_slot_cnt_x, k_player_inventory_slot_cnt_y};

    if (cursor_position_rel_to_inventory_top_left.x >= 0.0f && cursor_position_rel_to_inventory_top_left.y >= 0.0f && cursor_position_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_position_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_position_in_grid = {
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.x / k_player_inventory_ui_slot_distance)),
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.y / k_player_inventory_ui_slot_distance)),
        };

        if (slot_position_in_grid.y == 0 || inventory_open) {
            const zcl::t_v2 cursor_position_rel_to_slot_region = cursor_position_rel_to_inventory_top_left - (zcl::V2IToF(slot_position_in_grid) * k_player_inventory_ui_slot_distance);

            if (cursor_position_rel_to_slot_region.x < k_player_inventory_ui_slot_size && cursor_position_rel_to_slot_region.y < k_player_inventory_ui_slot_size) {
                return (slot_position_in_grid.y * k_player_inventory_slot_cnt_x) + slot_position_in_grid.x;
            }
        }
    }

    return -1;
}

t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i window_framebuffer_size, zcl::t_arena *const temp_arena) {
    t_world_tick_result_id result_id = ek_world_tick_result_id_normal;

    // @todo: Scroll support.

    for (zcl::t_i32 i = 0; i < k_player_inventory_slot_cnt_x; i++) {
        if (zgl::KeyCheckPressed(input_state, static_cast<zgl::t_key_code>(zgl::ek_key_code_1 + i))) {
            world->player_meta.inventory_hotbar_slot_selected_index = i;
            break;
        }
    }

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        world->player_meta.inventory_open = !world->player_meta.inventory_open;
    }

    if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
        const zcl::t_i32 slot_hovered_index = PlayerInventoryUIGetHoveredSlotIndex(zgl::CursorGetPos(input_state), world->player_meta.inventory_open);

        if (slot_hovered_index != -1) {
            const auto slot = InventoryGet(world->player_meta.inventory, slot_hovered_index);

            if (world->player_meta.cursor_held_quantity == 0) {
                world->player_meta.cursor_held_item_type_id = slot.item_type_id;
                world->player_meta.cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(world->player_meta.inventory, slot_hovered_index, slot.quantity);
            } else {
                if (slot.quantity == 0) {
                    InventoryAddAt(world->player_meta.inventory, slot_hovered_index, world->player_meta.cursor_held_item_type_id, world->player_meta.cursor_held_quantity);
                    world->player_meta.cursor_held_quantity = 0;
                }
            }
        }
    }

    PlayerProcessMovement(&world->player, &world->tilemap, input_state);
    CameraMove(&world->camera, world->player.position);

    return result_id;
}

void WorldRender(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const zcl::t_v2_i backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, backbuffer_size / 2);
    zgl::RendererPassBeginOffscreen(rendering_context, world->texture_target, camera_view_matrix, true, k_bg_color);

    TilemapRender(&world->tilemap, CameraCalcTilemapRect(world->camera, backbuffer_size), rendering_context, assets);

    PlayerRender(&world->player, rendering_context, assets);

    zgl::RendererPassEnd(rendering_context);

    zgl::RendererPassBegin(rendering_context, backbuffer_size);

    zgl::RendererSubmitTexture(rendering_context, world->texture_target, {}, {}, zcl::k_origin_top_left, 0.0f, {2.0f, 2.0f});

    zgl::RendererPassEnd(rendering_context);
}

static void RenderItemUI(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rendering_context, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    SpriteRender(g_item_types[item_type_id].icon_sprite_id, rendering_context, assets, pos, zcl::k_origin_center, 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rendering_context, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *GetFont(assets, ek_font_id_eb_garamond_24), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const auto backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

    //
    // Inventory
    //
    const zcl::t_i32 slot_cnt_y = world->player_meta.inventory_open ? k_player_inventory_slot_cnt_y : 1;

    for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
        for (zcl::t_i32 slot_x = 0; slot_x < k_player_inventory_slot_cnt_x; slot_x++) {
            const zcl::t_i32 slot_index = (slot_y * k_player_inventory_slot_cnt_x) + slot_x;

            const auto slot = InventoryGet(world->player_meta.inventory, slot_index);

            const auto ui_slot_rect = PlayerInventoryUISlotCalcRect(slot_index);

            const auto ui_slot_color = slot_y == 0 && world->player_meta.inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
            ZCL_ASSERT(ui_slot_color.a == 1.0f);

            zgl::RendererSubmitRect(rendering_context, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_player_inventory_ui_slot_bg_alpha));
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
    if (world->player_meta.cursor_held_quantity > 0) {
        RenderItemUI(world->player_meta.cursor_held_item_type_id, world->player_meta.cursor_held_quantity, rendering_context, zgl::CursorGetPos(input_state), assets, temp_arena);
    }
}
