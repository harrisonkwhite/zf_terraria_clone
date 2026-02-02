#include "world.h"

#include "camera.h"
#include "inventory.h"
#include "tiles.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_i32 k_player_inventory_slot_cnt = 28;

constexpr zcl::t_f32 k_gravity = 0.2f;

constexpr zcl::t_f32 k_player_entity_move_spd = 1.5f;
constexpr zcl::t_f32 k_player_entity_move_spd_acc = 0.2f;
constexpr zcl::t_f32 k_player_entity_jump_height = 3.5f;
constexpr zcl::t_v2 k_player_entity_origin = zcl::k_origin_center;

constexpr zcl::t_f32 k_ui_tile_highlight_alpha = 0.6f;

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

    zcl::t_i32 item_use_time;
};

constexpr zcl::t_i32 k_pop_up_death_time_limit = 15;
constexpr zcl::t_f32 k_pop_up_lerp_factor = 0.15f;

struct t_pop_up {
    zcl::t_v2 pos;
    zcl::t_v2 vel;

    zcl::t_i32 death_time;

    zcl::t_static_array<zcl::t_u8, 32> str_bytes;
    zcl::t_i32 str_byte_cnt;

    t_font_id font_id;
};

constexpr zcl::t_i32 k_pop_up_limit = 1024;

struct t_pop_ups {
    zcl::t_static_array<t_pop_up, k_pop_up_limit> buf;
    zcl::t_static_bitset<k_pop_up_limit> activity;
};

struct t_world {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?

    zcl::t_i32 player_health;
    zcl::t_i32 player_health_limit;

    t_inventory *player_inventory;

    t_tilemap *tilemap;

    t_player_entity player_entity;

    t_camera *camera;

    t_pop_ups pop_ups;

    struct {
        zcl::t_i32 player_inventory_open;
        zcl::t_i32 player_inventory_hotbar_slot_selected_index;

        t_item_type_id cursor_held_item_type_id;
        zcl::t_i32 cursor_held_quantity;
    } ui;
};

// Data relevant to the item type only in the context of the world phase.
struct t_item_type_info_world {
    zcl::t_i32 use_time;                         // The length of the break in ticks between each item use.
    zcl::t_b8 use_consume;                       // Does the item get removed from inventory on use?
    zcl::t_b8 (*use_func)(t_world *const world); // Called when the item is used. Should return true iff the item was successfully used (this info is needed to determine whether to consume the item for example).
};

static const zcl::t_static_array<t_item_type_info_world, ekm_item_type_id_cnt> g_item_type_infos_world = {{
    {
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_func = [](t_world *const world) {
            zcl::Log(ZCL_STR_LITERAL("use!"));
            return true;
        },
    },
    {
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_func = [](t_world *const world) {
            zcl::Log(ZCL_STR_LITERAL("use!"));
            return true;
        },
    },
    {
        .use_time = k_item_type_default_block_use_time,
        .use_consume = true,
        .use_func = [](t_world *const world) {
            zcl::Log(ZCL_STR_LITERAL("use!"));
            return true;
        },
    },
}}; // @todo: Some way to static assert that all array elements have been set! This is for any static array!

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

static void ProcessTilemapCollisionsVertical(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
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

    ProcessTilemapCollisionsVertical(pos, &vel->y, collider_size, collider_origin, tilemap);

    const zcl::t_rect_f collider_diagonal = ColliderCreate(*pos + *vel, collider_size, collider_origin);

    if (TilemapCheckCollision(tilemap, collider_diagonal)) {
        vel->x = 0.0f;
    }
}

static zcl::t_v2_i ScreenToTilemapPos(const zcl::t_v2 pos, const zcl::t_v2_i screen_size, const t_camera *const camera) {
    return zcl::V2FToI(ScreenToCameraPos(pos, screen_size, camera) / k_tile_size);
}

static void UIRenderItem(const t_item_type_id item_type_id, const zcl::t_i32 quantity, const zgl::t_rendering_context rc, const zcl::t_v2 pos, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    ZCL_ASSERT(quantity > 0);

    SpriteRender(g_item_type_infos_basic[item_type_id].icon_sprite_id, rc, assets, pos, zcl::k_origin_center, 0.0f, {2.0f, 2.0f});

    if (quantity > 1) {
        zcl::t_static_array<zcl::t_u8, 32> quantity_str_bytes;
        auto quantity_str_bytes_stream = zcl::ByteStreamCreate(quantity_str_bytes, zcl::ek_stream_mode_write);
        zcl::PrintFormat(zcl::ByteStreamGetView(&quantity_str_bytes_stream), ZCL_STR_LITERAL("x%"), quantity);

        zgl::RendererSubmitStr(rc, {zcl::ByteStreamGetWritten(&quantity_str_bytes_stream)}, *GetFont(assets, ek_font_id_eb_garamond_24), pos, zcl::k_color_white, temp_arena, zcl::k_origin_top_left);
    }
}

// Returns -1 if no slot is hovered.
static zcl::t_i32 UIPlayerInventoryGetHoveredSlotIndex(const zcl::t_v2 cursor_position, const zcl::t_b8 inventory_open) {
    const zcl::t_v2 cursor_position_rel_to_inventory_top_left = cursor_position - k_ui_player_inventory_offs_top_left;
    const zcl::t_v2 inventory_size_in_pixels = k_ui_player_inventory_slot_distance * zcl::t_v2{k_ui_player_inventory_slot_cnt_x, k_ui_player_inventory_slot_cnt_y};

    if (cursor_position_rel_to_inventory_top_left.x >= 0.0f && cursor_position_rel_to_inventory_top_left.y >= 0.0f && cursor_position_rel_to_inventory_top_left.x < inventory_size_in_pixels.x && cursor_position_rel_to_inventory_top_left.y < inventory_size_in_pixels.y) {
        const zcl::t_v2_i slot_position_in_grid = {
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.x / k_ui_player_inventory_slot_distance)),
            static_cast<zcl::t_i32>(floor(cursor_position_rel_to_inventory_top_left.y / k_ui_player_inventory_slot_distance)),
        };

        if (slot_position_in_grid.y == 0 || inventory_open) {
            const zcl::t_v2 cursor_position_rel_to_slot_region = cursor_position_rel_to_inventory_top_left - (zcl::V2IToF(slot_position_in_grid) * k_ui_player_inventory_slot_distance);

            if (cursor_position_rel_to_slot_region.x < k_ui_player_inventory_slot_size && cursor_position_rel_to_slot_region.y < k_ui_player_inventory_slot_size) {
                return (slot_position_in_grid.y * k_ui_player_inventory_slot_cnt_x) + slot_position_in_grid.x;
            }
        }
    }

    return -1;
}

static zcl::t_rect_f UIPlayerInventoryCalcSlotRect(const zcl::t_i32 slot_index) {
    ZCL_ASSERT(slot_index >= 0 && slot_index < k_player_inventory_slot_cnt);

    const zcl::t_v2_i slot_pos = {slot_index % k_ui_player_inventory_slot_cnt_x, slot_index / k_ui_player_inventory_slot_cnt_x};
    const zcl::t_v2 ui_slot_pos = k_ui_player_inventory_offs_top_left + (zcl::t_v2{static_cast<zcl::t_f32>(slot_pos.x), static_cast<zcl::t_f32>(slot_pos.y)} * k_ui_player_inventory_slot_distance);
    const zcl::t_v2 ui_slot_size = {k_ui_player_inventory_slot_size, k_ui_player_inventory_slot_size};

    return zcl::RectCreateF(ui_slot_pos, ui_slot_size);
}

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

static void PlayerEntityRender(const t_player_entity *const player, const zgl::t_rendering_context rc, const t_assets *const assets) {
    SpriteRender(ek_sprite_id_player, rc, assets, {player->pos.x, player->pos.y}, k_player_entity_origin);
}

static t_pop_up *PopUpSpawn(t_pop_ups *const pop_ups, const zcl::t_v2 pos, const zcl::t_v2 vel, const t_font_id font_id = ek_font_id_eb_garamond_32) {
    const zcl::t_i32 index = zcl::BitsetFindFirstUnset(pop_ups->activity);

    ZCL_REQUIRE(index != -1);

    pop_ups->buf[index] = {
        .pos = pos,
        .vel = vel,
        .font_id = font_id,
    };

    zcl::BitsetSet(pop_ups->activity, index);

    return &pop_ups->buf[index];
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

    result->player_health_limit = 100;
    result->player_health = result->player_health_limit;

    result->player_inventory = InventoryCreate(k_player_inventory_slot_cnt, arena);
    InventoryAdd(result->player_inventory, ek_item_type_id_dirt_block, 2);

    result->tilemap = WorldGen(result->rng, arena);

    const zcl::t_v2 world_size = zcl::V2IToF(k_tilemap_size * k_tile_size);
    result->player_entity.pos = MakeContactWithTilemap({world_size.x * 0.5f, 0.0f}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(PlayerEntityColliderCreate(result->player_entity.pos)), k_player_entity_origin, result->tilemap);

    result->camera = CameraCreate(result->player_entity.pos, 2.0f, 0.3f, arena);

    return result;
}

t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
    t_world_tick_result_id result_id = ek_world_tick_result_id_normal;

    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    // ----------------------------------------
    // Player Inventory Hotbar

    for (zcl::t_i32 i = 0; i < k_ui_player_inventory_slot_cnt_x; i++) {
        if (zgl::KeyCheckPressed(input_state, static_cast<zgl::t_key_code>(zgl::ek_key_code_1 + i))) {
            world->ui.player_inventory_hotbar_slot_selected_index = i;
            break;
        }
    }

    {
        const zcl::t_v2 scroll_offs = zgl::ScrollGetOffset(input_state);

        if (scroll_offs.y != 0.0f) {
            world->ui.player_inventory_hotbar_slot_selected_index += round(scroll_offs.y);
            world->ui.player_inventory_hotbar_slot_selected_index = zcl::Wrap(world->ui.player_inventory_hotbar_slot_selected_index, 0, k_ui_player_inventory_slot_cnt_x);
        }
    }

    // ------------------------------

    // ----------------------------------------
    // Player Inventory Interaction

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        world->ui.player_inventory_open = !world->ui.player_inventory_open;
    }

    if (zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left)) {
        const zcl::t_i32 slot_hovered_index = UIPlayerInventoryGetHoveredSlotIndex(cursor_pos, world->ui.player_inventory_open);

        if (slot_hovered_index != -1) {
            const auto slot = InventoryGet(world->player_inventory, slot_hovered_index);

            if (world->ui.cursor_held_quantity == 0) {
                world->ui.cursor_held_item_type_id = slot.item_type_id;
                world->ui.cursor_held_quantity = slot.quantity;

                InventoryRemoveAt(world->player_inventory, slot_hovered_index, slot.quantity);
            } else {
                if (slot.quantity == 0) {
                    InventoryAddAt(world->player_inventory, slot_hovered_index, world->ui.cursor_held_item_type_id, world->ui.cursor_held_quantity);
                    world->ui.cursor_held_quantity = 0;
                }
            }
        }
    }

    // ------------------------------

    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_x)) {
        const auto pop_up = PopUpSpawn(&world->pop_ups, world->player_entity.pos, {0.0f, -6.0f});

        // @todo: This is pretty annoying to do, maybe in ZF there could be better abstractions for this?
        zcl::t_byte_stream str_bytes_stream = zcl::ByteStreamCreate(pop_up->str_bytes, zcl::ek_stream_mode_write);
        zcl::Print(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("YEAH"));
        pop_up->str_byte_cnt = zcl::ByteStreamGetWritten(&str_bytes_stream).len;
    }

    PlayerEntityProcessMovement(&world->player_entity, world->tilemap, input_state); // @note: For a function like this, what if you just had a lambda that gets called and is exposed a subset of state?

    // ----------------------------------------
    // Item Usage

    if (world->player_entity.item_use_time > 0) {
        world->player_entity.item_use_time--;
    } else {
        if (zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left)) {
            const t_inventory_slot hotbar_slot_selected = InventoryGet(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index);
            const t_item_type_id item_type_id = hotbar_slot_selected.item_type_id;

            if (hotbar_slot_selected.quantity > 0) {
#if 0
                zcl::t_b8 item_used = false;

                {
                    const zcl::t_v2_i tile_hovered_pos = ScreenToTilemapPos(cursor_pos, screen_size, world->camera);

                    if (!TilemapCheck(world->tilemap, tile_hovered_pos)) {
                        TilemapAdd(world->tilemap, tile_hovered_pos, ek_tile_type_id_dirt);
                        item_used = true;
                    }
                }

                if (item_used) {
                    if (g_item_type_infos_world[item_type_id].use_func) { // @note: Maybe it should be mandatory?
                        if (g_item_type_infos_world[item_type_id].use_func(world)) {
                        }
                    }

                    if (g_item_type_infos_world[item_type_id].use_consume) {
                        InventoryRemoveAt(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index, 1);
                    }
                }
#endif

                if (g_item_type_infos_world[item_type_id].use_func) { // @note: Maybe it should be mandatory?
                    const zcl::t_b8 item_used = g_item_type_infos_world[item_type_id].use_func(world);

                    if (item_used) {
                        if (g_item_type_infos_world[item_type_id].use_consume) {
                            InventoryRemoveAt(world->player_inventory, world->ui.player_inventory_hotbar_slot_selected_index, 1);
                        }

                        world->player_entity.item_use_time = g_item_type_infos_world[hotbar_slot_selected.item_type_id].use_time;
                    }
                }
            }
        }
    }

    // ------------------------------

    CameraMove(world->camera, world->player_entity.pos);

    // ----------------------------------------
    // Updating Pop-Ups

    [pop_ups = &world->pop_ups]() {
        ZCL_BITSET_WALK_ALL_SET (pop_ups->activity, i) {
            const auto pop_up = &pop_ups->buf[i];

            pop_up->pos += pop_up->vel;
            pop_up->vel = zcl::Lerp(pop_up->vel, {}, k_pop_up_lerp_factor);

            if (zcl::CheckNearlyEqual(pop_up->vel, {}, 0.01f)) {
                if (pop_up->death_time < k_pop_up_death_time_limit) {
                    pop_up->death_time++;
                } else {
                    zcl::BitsetUnset(pop_ups->activity, i);
                }
            }
        }
    }();

    // ------------------------------

    return result_id;
}

static zcl::t_rect_i CalcCameraTilemapRect(const t_camera *const camera, const zcl::t_v2_i screen_size) {
    const zcl::t_f32 camera_scale = CameraGetScale(camera);

    const zcl::t_rect_f camera_rect = CameraCalcRect(camera, screen_size);

    const zcl::t_i32 camera_tilemap_left = static_cast<zcl::t_i32>(floor(zcl::RectGetLeft(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_top = static_cast<zcl::t_i32>(floor(zcl::RectGetTop(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_right = static_cast<zcl::t_i32>(ceil(zcl::RectGetRight(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_bottom = static_cast<zcl::t_i32>(ceil(zcl::RectGetBottom(camera_rect) / k_tile_size));

    return zcl::ClampWithinContainer(zcl::RectCreateI(camera_tilemap_left, camera_tilemap_top, camera_tilemap_right - camera_tilemap_left, camera_tilemap_bottom - camera_tilemap_top), zcl::RectCreateI({}, k_tilemap_size));
}

void WorldRender(const t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, rc.screen_size);
    zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix, true, k_bg_color);

    TilemapRender(world->tilemap, CalcCameraTilemapRect(world->camera, rc.screen_size), rc, assets);

    PlayerEntityRender(&world->player_entity, rc, assets);

    zgl::RendererPassEnd(rc);
}

void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

    // ----------------------------------------
    // Tile Highlight

    {
        const zcl::t_v2_i tile_hovered_pos = ScreenToTilemapPos(cursor_pos, rc.screen_size, world->camera);
        const zcl::t_v2 tile_hovered_pos_world = zcl::V2IToF(tile_hovered_pos) * k_tile_size;

        const zcl::t_rect_f rect = zcl::RectCreateF(CameraToScreenPos(tile_hovered_pos_world, world->camera, rc.screen_size), zcl::t_v2{k_tile_size, k_tile_size} * CameraGetScale(world->camera));

        zgl::RendererSubmitRect(rc, rect, zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, k_ui_tile_highlight_alpha));
    }

    // ------------------------------

    // ----------------------------------------
    // Pop-Ups

    ZCL_BITSET_WALK_ALL_SET (world->pop_ups.activity, i) {
        const auto pop_up = &world->pop_ups.buf[i];

        const zcl::t_f32 life_perc = 1.0f - (static_cast<zcl::t_f32>(pop_up->death_time) / k_pop_up_death_time_limit);

        zgl::RendererSubmitStr(rc, {{pop_up->str_bytes.raw, pop_up->str_byte_cnt}}, *GetFont(assets, pop_up->font_id), CameraToScreenPos(pop_up->pos, world->camera, rc.screen_size), zcl::ColorCreateRGBA32F(1.0f, 1.0f, 1.0f, life_perc), temp_arena, zcl::k_origin_center, 0.0f, {life_perc, life_perc});
    }

    // ------------------------------

    // ----------------------------------------
    // Inventory

    {
        const zcl::t_i32 slot_cnt_y = world->ui.player_inventory_open ? k_ui_player_inventory_slot_cnt_y : 1;

        for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
            for (zcl::t_i32 slot_x = 0; slot_x < k_ui_player_inventory_slot_cnt_x; slot_x++) {
                const zcl::t_i32 slot_index = (slot_y * k_ui_player_inventory_slot_cnt_x) + slot_x;

                const auto slot = InventoryGet(world->player_inventory, slot_index);

                const auto ui_slot_rect = UIPlayerInventoryCalcSlotRect(slot_index);

                const auto ui_slot_color = slot_y == 0 && world->ui.player_inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
                ZCL_ASSERT(ui_slot_color.a == 1.0f);

                zgl::RendererSubmitRect(rc, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_ui_player_inventory_slot_bg_alpha));
                zgl::RendererSubmitRectOutlineOpaque(rc, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

                if (slot.quantity > 0) {
                    UIRenderItem(slot.item_type_id, slot.quantity, rc, zcl::RectGetCenter(ui_slot_rect), assets, temp_arena);
                }
            }
        }
    }

    // ------------------------------

    // --------------------------------------------------
    // Health

    {
        const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(rc.screen_size.x) - k_ui_player_health_bar_offs_top_right.x - k_ui_player_health_bar_size.x, k_ui_player_health_bar_offs_top_right.y};
        const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_ui_player_health_bar_size);

        zgl::RendererSubmitRectOutlineOpaque(rc, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);
    }

    // ------------------------------

    // --------------------------------------------------
    // Cursor Held

    if (world->ui.cursor_held_quantity > 0) {
        UIRenderItem(world->ui.cursor_held_item_type_id, world->ui.cursor_held_quantity, rc, cursor_pos, assets, temp_arena);
    }

    // ------------------------------
}
