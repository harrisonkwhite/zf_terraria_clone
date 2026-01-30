#include "world.h"

#include "sprites.h"
#include "inventory.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_gravity = 0.2f;

static zcl::t_rect_f ColliderCreate(const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin) {
    ZCL_ASSERT(size.x > 0.0f && size.y > 0.0f);
    ZCL_ASSERT(zcl::OriginCheckValid(origin));

    return zcl::RectCreateF(pos - zcl::CalcCompwiseProd(size, origin), size);
}

static zcl::t_rect_f ColliderCreateFromSprite(const t_sprite_id sprite_id, const zcl::t_v2 pos, const zcl::t_v2 origin) {
    return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[sprite_id].src_rect)), origin);
}


// ============================================================
// @section: Camera
// ============================================================

constexpr zcl::t_f32 k_camera_lerp_factor = 0.3f;

struct t_camera {
    zcl::t_v2 position;
};

static zcl::t_f32 CameraCalcScale(const zcl::t_v2_i backbuffer_size) {
    return backbuffer_size.x > 1600 || backbuffer_size.y > 900 ? 3.0f : 2.0f;
}

static zcl::t_rect_f CameraCalcRect(const t_camera camera, const zcl::t_v2_i backbuffer_size) {
    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);

    return {
        .x = camera.position.x - (backbuffer_size.x / (2.0f * camera_scale)),
        .y = camera.position.y - (backbuffer_size.y / (2.0f * camera_scale)),
        .width = backbuffer_size.x / camera_scale,
        .height = backbuffer_size.y / camera_scale,
    };
}

static zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera camera, const zcl::t_v2_i backbuffer_size) {
    ZCL_ASSERT(backbuffer_size.x > 0 && backbuffer_size.y > 0);

    zcl::t_mat4x4 result = zcl::MatrixCreateIdentity();

    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateScaled({camera_scale, camera_scale}));

    result = zcl::MatrixMultiply(result, zcl::MatrixCreateTranslated((-camera.position * camera_scale) + (zcl::V2IToF(backbuffer_size) / 2.0f)));

    return result;
}

static void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ) {
    camera->position = zcl::Lerp(camera->position, pos_targ, k_camera_lerp_factor);
}

// ============================================================


// ============================================================
// @section: Tiles and Tilemap
// ============================================================

struct t_tile_type {
    t_sprite_id sprite;
};

enum t_tile_type_id : zcl::t_i8 {
    ek_tile_type_id_dirt,
    ek_tile_type_id_stone,
    ek_tile_type_id_grass,

    ekm_tile_type_id_cnt
};

constexpr zcl::t_static_array<t_tile_type, ekm_tile_type_id_cnt> k_tile_types = {{
    {.sprite = ek_sprite_id_dirt_tile},
    {.sprite = ek_sprite_id_stone_tile},
    {.sprite = ek_sprite_id_grass_tile},
}};

constexpr zcl::t_i32 k_tile_size = 8;

constexpr zcl::t_v2_i k_tilemap_size = {4000, 800};

struct t_tilemap {
    zcl::t_static_bitset<k_tilemap_size.x * k_tilemap_size.y> activity;
    zcl::t_static_array<zcl::t_static_array<t_tile_type_id, k_tilemap_size.x>, k_tilemap_size.y> types;
};

static zcl::t_b8 TilePosCheckInBounds(const zcl::t_v2_i pos) {
    return pos.x >= 0 && pos.x < k_tilemap_size.x && pos.y >= 0 && pos.y < k_tilemap_size.y;
}

static void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
    ZCL_ASSERT(TilePosCheckInBounds(tile_pos));

    zcl::BitsetSet(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
    tm->types[tile_pos.y][tile_pos.x] = tile_type;
}

static void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilePosCheckInBounds(tile_pos));
    zcl::BitsetUnset(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
}

static zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilePosCheckInBounds(tile_pos));
    return zcl::BitsetCheckSet(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
}

static zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect) {
    const zcl::t_i32 left = static_cast<zcl::t_i32>(floor(rect.x / k_tile_size));
    const zcl::t_i32 top = static_cast<zcl::t_i32>(floor(rect.y / k_tile_size));
    const zcl::t_i32 right = static_cast<zcl::t_i32>(ceil(zcl::RectGetRight(rect) / k_tile_size));
    const zcl::t_i32 bottom = static_cast<zcl::t_i32>(ceil(zcl::RectGetBottom(rect) / k_tile_size));

    const zcl::t_rect_i result_without_clamp = {
        left,
        top,
        right - left,
        bottom - top,
    };

    return zcl::ClampWithinContainer(result_without_clamp, zcl::RectCreateI({}, k_tilemap_size));
}

static zcl::t_b8 TileCollisionCheck(const t_tilemap *const tilemap, const zcl::t_rect_f collider) {
    const zcl::t_rect_i collider_tilemap_span = TilemapCalcRectSpan(collider);

    for (zcl::t_i32 ty = zcl::RectGetTop(collider_tilemap_span); ty < zcl::RectGetBottom(collider_tilemap_span); ty++) {
        for (zcl::t_i32 tx = zcl::RectGetLeft(collider_tilemap_span); tx < zcl::RectGetRight(collider_tilemap_span); tx++) {
            if (!TilemapCheck(tilemap, {tx, ty})) {
                continue;
            }

            const zcl::t_rect_f tile_collider = {
                static_cast<zcl::t_f32>(k_tile_size * tx),
                static_cast<zcl::t_f32>(k_tile_size * ty),
                k_tile_size,
                k_tile_size,
            };

            if (zcl::CheckInters(collider, tile_collider)) {
                return true;
            }
        }
    }

    return false;
}

static zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    ZCL_ASSERT(jump_size > 0.0f);

    zcl::t_v2 pos_next = pos_current;

    const zcl::t_v2 jump_dir = zcl::k_cardinal_direction_normals[cardinal_dir_id];
    const zcl::t_v2 jump = jump_dir * jump_size;

    while (!TileCollisionCheck(tilemap, ColliderCreate(pos_next + jump, collider_size, collider_origin))) {
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

    if (TileCollisionCheck(tilemap, collider_vertical)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_jump_size_precise, *vel_y >= 0.0f ? zcl::ek_cardinal_direction_down : zcl::ek_cardinal_direction_up, collider_size, collider_origin, tilemap);
        *vel_y = 0.0f;
    }
}

static void ProcessTileCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_hor = ColliderCreate({pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, collider_hor)) {
        *pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_jump_size_precise, vel->x >= 0.0f ? zcl::ek_cardinal_direction_right : zcl::ek_cardinal_direction_left, collider_size, collider_origin, tilemap);
        vel->x = 0.0f;
    }

    ProcessTileCollisionsVertical(pos, &vel->y, collider_size, collider_origin, tilemap);

    const zcl::t_rect_f collider_diagonal = ColliderCreate(*pos + *vel, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, collider_diagonal)) {
        vel->x = 0.0f;
    }
}

static void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rendering_context, const t_assets *const assets) {
    ZCL_ASSERT(zcl::CheckRectInRect(tm_subset, zcl::RectCreateI(0, 0, k_tilemap_size.x, k_tilemap_size.y)));

    for (zcl::t_i32 ty = zcl::RectGetTop(tm_subset); ty < zcl::RectGetBottom(tm_subset); ty++) {
        for (zcl::t_i32 tx = zcl::RectGetLeft(tm_subset); tx < zcl::RectGetRight(tm_subset); tx++) {
            if (!TilemapCheck(tm, {tx, ty})) {
                continue;
            }

            const t_tile_type_id tile_type_id = tm->types[ty][tx];
            const t_tile_type *const tile_type_info = &k_tile_types[tile_type_id];

            const zcl::t_v2 tile_world_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

            SpriteRender(tile_type_info->sprite, rendering_context, assets, tile_world_pos);
        }
    }
}

// ============================================================


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

struct t_player_meta {
    zcl::t_b8 dead;

    t_inventory *inventory;
    zcl::t_i32 inventory_open;
    zcl::t_i32 inventory_hotbar_slot_selected_index;

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

static void PlayerMetaUIRender(const t_player_meta *const meta, const zgl::t_rendering_context rendering_context, const t_assets *const assets) {
    const auto backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

    //
    // Inventory
    //
    const zcl::t_i32 slot_cnt_y = meta->inventory_open ? k_player_inventory_slot_cnt_y : 1;

    for (zcl::t_i32 slot_y = 0; slot_y < slot_cnt_y; slot_y++) {
        for (zcl::t_i32 slot_x = 0; slot_x < k_player_inventory_slot_cnt_x; slot_x++) {
            const zcl::t_i32 slot_index = (slot_y * k_player_inventory_slot_cnt_x) + slot_x;

            const auto slot = InventoryGet(meta->inventory, slot_index);

            const auto ui_slot_rect = PlayerInventoryUISlotCalcRect(slot_index);

            const auto ui_slot_color = slot_y == 0 && meta->inventory_hotbar_slot_selected_index == slot_x ? zcl::k_color_yellow : zcl::k_color_white;
            ZCL_ASSERT(ui_slot_color.a == 1.0f);

            zgl::RendererSubmitRect(rendering_context, ui_slot_rect, zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_player_inventory_ui_slot_bg_alpha));
            zgl::RendererSubmitRectOutlineOpaque(rendering_context, ui_slot_rect, ui_slot_color.r, ui_slot_color.g, ui_slot_color.b, 0.0f, 2.0f);

            if (slot.quantity > 0) {
                SpriteRender(g_item_types[slot.item_type_id].icon_sprite_id, rendering_context, assets, zcl::RectGetCenter(ui_slot_rect), zcl::k_origin_center, 0.0f, {2.0f, 2.0f});
            }
        }
    }

    //
    // Health
    //
    const zcl::t_v2 health_bar_pos = {static_cast<zcl::t_f32>(backbuffer_size.x) - k_player_health_bar_offs_top_right.x - k_player_health_bar_size.x, k_player_health_bar_offs_top_right.y};
    const auto health_bar_rect = zcl::RectCreateF(health_bar_pos, k_player_health_bar_size);

    zgl::RendererSubmitRectOutlineOpaque(rendering_context, health_bar_rect, 1.0f, 1.0f, 1.0f, 0.0f, 2.0f);
}

// ============================================================


struct t_world {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?
    t_player_meta player_meta;
    t_camera camera;
    t_tilemap tilemap;
    t_player player;
};

static void WorldGen(zcl::t_rng *const rng, t_tilemap *const o_tilemap) {
    zcl::ZeroClearItem(o_tilemap);

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
                TilemapAdd(o_tilemap, {x, ground_tilemap_y_begin + gy}, ek_tile_type_id_dirt);
            }
        }
    }

    for (zcl::t_i32 y = ground_tilemap_y_begin + k_ground_height; y < k_tilemap_size.y; y++) {
        for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
            TilemapAdd(o_tilemap, {x, y}, ek_tile_type_id_dirt);
        }
    }
}

t_world *WorldCreate(zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_world>(arena);

    result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

    result->player_meta.inventory = InventoryCreate(k_player_inventory_slot_cnt, arena);
    InventoryAdd(result->player_meta.inventory, ek_item_type_id_dirt_block, 1);

    WorldGen(result->rng, &result->tilemap);

    const zcl::t_v2 world_size = zcl::V2IToF(k_tilemap_size * k_tile_size);
    result->player.position = MakeContactWithTilemap({world_size.x * 0.5f, 0.0f}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(PlayerColliderCreate(result->player.position)), k_player_origin, &result->tilemap);

    result->camera.position = result->player.position;

    return result;
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

    {
        const zcl::t_v2 cursor_position = zgl::CursorGetPos(input_state);
        const zcl::t_v2 cursor_position_relative = cursor_position - k_player_inventory_ui_offs_top_left;

        const zcl::t_v2 player_inventory_ui_size = k_player_inventory_ui_slot_distance * zcl::t_v2{k_player_inventory_slot_cnt_x, k_player_inventory_slot_cnt_y};

        if (cursor_position_relative.x >= 0.0f && cursor_position_relative.y >= 0.0f && cursor_position_relative.x < player_inventory_ui_size.x && cursor_position_relative.y < player_inventory_ui_size.y) {
            const zcl::t_v2_i slot_position = {
                static_cast<zcl::t_i32>(floor(cursor_position_relative.x / k_player_inventory_ui_slot_distance)),
                static_cast<zcl::t_i32>(floor(cursor_position_relative.y / k_player_inventory_ui_slot_distance)),
            };

            zcl::Log(ZCL_STR_LITERAL("Slot pos: %"), slot_position);
        }
    }

    PlayerProcessMovement(&world->player, &world->tilemap, input_state);
    CameraMove(&world->camera, world->player.position);

    return result_id;
}

static zcl::t_rect_i CalcCameraTilemapRect(const t_camera camera, const zcl::t_v2_i backbuffer_size) {
    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);

    const zcl::t_rect_f camera_rect = CameraCalcRect(camera, backbuffer_size);

    const zcl::t_i32 camera_tilemap_left = static_cast<zcl::t_i32>(floor(zcl::RectGetLeft(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_top = static_cast<zcl::t_i32>(floor(zcl::RectGetTop(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_right = static_cast<zcl::t_i32>(ceil(zcl::RectGetRight(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_bottom = static_cast<zcl::t_i32>(ceil(zcl::RectGetBottom(camera_rect) / k_tile_size));

    return zcl::ClampWithinContainer(zcl::RectCreateI(camera_tilemap_left, camera_tilemap_top, camera_tilemap_right - camera_tilemap_left, camera_tilemap_bottom - camera_tilemap_top), zcl::RectCreateI({}, k_tilemap_size));
}

void WorldRender(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const zcl::t_v2_i backbuffer_size = zgl::BackbufferGetSize(rendering_context.gfx_ticket);

    const auto camera_view_matrix = CameraCalcViewMatrix(world->camera, backbuffer_size);
    zgl::RendererPassBegin(rendering_context, backbuffer_size, camera_view_matrix, true, k_bg_color);

    TilemapRender(&world->tilemap, CalcCameraTilemapRect(world->camera, backbuffer_size), rendering_context, assets);

    PlayerRender(&world->player, rendering_context, assets);

    zgl::RendererPassEnd(rendering_context);
}

void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    PlayerMetaUIRender(&world->player_meta, rendering_context, assets);
}
