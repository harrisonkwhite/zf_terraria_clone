#include "world.h"

#include "sprites.h"

constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

constexpr zcl::t_f32 k_pause_bg_darkness_alpha = 0.2f;

constexpr zcl::t_f32 k_gravity = 0.2f;

static zcl::t_rect_f ColliderCreate(const zcl::t_v2 pos, const zcl::t_v2 size, const zcl::t_v2 origin) {
    ZCL_ASSERT(size.x > 0.0f && size.y > 0.0f);
    ZCL_ASSERT(zcl::OriginCheckValid(origin));

    return zcl::RectCreateF(pos - zcl::CalcCompwiseProd(size, origin), size);
}

static zcl::t_rect_f ColliderCreateFromSprite(const t_sprite_id spr_id, const zcl::t_v2 pos, const zcl::t_v2 origin) {
    return ColliderCreate(pos, zcl::V2IToF(zcl::RectGetSize(k_sprites[spr_id].src_rect)), origin);
}


// ============================================================
// @section: Camera
// ============================================================

constexpr zcl::t_f32 k_camera_lerp_factor = 0.3f;

struct t_camera {
    zcl::t_v2 pos;
};

// @todo: window_size is an awkward name.
static inline zcl::t_f32 CameraCalcScale(const zcl::t_v2_i window_size) {
    return window_size.x > 1600 || window_size.y > 900 ? 3.0f : 2.0f;
}

// @todo: window_size is an awkward name.
static zcl::t_mat4x4 CameraCreateViewMatrix(const t_camera cam, const zcl::t_v2_i window_size) {
    ZCL_ASSERT(window_size.x > 0 && window_size.y > 0);

    zcl::t_mat4x4 result = zcl::MatrixCreateIdentity();

    const zcl::t_f32 cam_scale = CameraCalcScale(window_size);
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateScaled({cam_scale, cam_scale}));

    result = zcl::MatrixMultiply(result, zcl::MatrixCreateTranslated((-cam.pos * cam_scale) + (zcl::V2IToF(window_size) / 2.0f)));

    return result;
}

static void CameraMove(t_camera *const cam, const zcl::t_v2 pos_targ) {
    cam->pos = zcl::Lerp(cam->pos, pos_targ, k_camera_lerp_factor);
}

// ============================================================


// ============================================================
// @section: Tiles and Tilemap
// ============================================================

struct t_tile_type_info {
    t_sprite_id spr;
};

enum t_tile_type_id : zcl::t_i8 {
    ek_tile_type_id_dirt,
    ek_tile_type_id_stone,
    ek_tile_type_id_grass,

    ekm_tile_type_id_cnt
};

constexpr zcl::t_static_array<t_tile_type_info, ekm_tile_type_id_cnt> k_tile_type_infos = {{
    {.spr = ek_sprite_id_dirt_tile},
    {.spr = ek_sprite_id_stone_tile},
    {.spr = ek_sprite_id_grass_tile},
}};

constexpr zcl::t_i32 k_tile_size = 8;

constexpr zcl::t_v2_i k_tilemap_size = {100, 100};

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

constexpr zcl::t_f32 k_tilemap_contact_precise_jump_size = 0.1f;

enum t_cardinal_direction_id : zcl::t_i32 {
    ek_cardinal_direction_up,
    ek_cardinal_direction_right,
    ek_cardinal_direction_down,
    ek_cardinal_direction_left,

    ekm_cardinal_direction_cnt
};

constexpr zcl::t_static_array<zcl::t_v2, ekm_cardinal_direction_cnt> k_cardinal_direction_normals = {{
    {0.0f, -1.0f},
    {1.0f, 0.0f},
    {0.0f, 1.0f},
    {-1.0f, 0.0f},
}};

static zcl::t_v2 MakeContactWithTilemapByJumpSize(const zcl::t_v2 pos_current, const zcl::t_f32 jump_size, const t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    ZCL_ASSERT(jump_size > 0.0f);

    zcl::t_v2 pos_next = pos_current;

    const zcl::t_v2 jump_dir = k_cardinal_direction_normals[cardinal_dir_id];
    const zcl::t_v2 jump = jump_dir * jump_size;

    while (!TileCollisionCheck(tilemap, ColliderCreate(pos_next + jump, collider_size, collider_origin))) {
        pos_next += jump;
    }

    return pos_next;
}

static zcl::t_v2 MakeContactWithTilemap(const zcl::t_v2 pos_current, const t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    zcl::t_v2 pos_next = pos_current;

    // Jump by tile intervals first, then make more precise contact.
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_tile_size, cardinal_dir_id, collider_size, collider_origin, tilemap);
    pos_next = MakeContactWithTilemapByJumpSize(pos_next, k_tilemap_contact_precise_jump_size, cardinal_dir_id, collider_size, collider_origin, tilemap);

    return pos_next;
}

static void ProcessTileCollisionsVertical(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_vertical = ColliderCreate({pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, collider_vertical)) {
        //*pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_precise_jump_size, *vel_y >= 0.0f ? ek_cardinal_direction_down : ek_cardinal_direction_up, collider_size, collider_origin, tilemap);
        *vel_y = 0.0f;
    }
}

static void ProcessTileCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f collider_hor = ColliderCreate({pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, collider_hor)) {
        //*pos = MakeContactWithTilemapByJumpSize(*pos, k_tilemap_contact_precise_jump_size, vel->x >= 0.0f ? ek_cardinal_direction_right : ek_cardinal_direction_left, collider_size, collider_origin, tilemap);
        vel->x = 0.0f;
    }

    ProcessTileCollisionsVertical(pos, &vel->y, collider_size, collider_origin, tilemap);

    const zcl::t_rect_f collider_diagonal = ColliderCreate(*pos + *vel, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, collider_diagonal)) {
        vel->x = 0.0f;
    }
}

void TilemapRender(const t_tilemap *const tm, const zgl::t_rendering_context rendering_context, const t_assets *const assets) {
    for (zcl::t_i32 ty = 0; ty < k_tilemap_size.y; ty++) {
        for (zcl::t_i32 tx = 0; tx < k_tilemap_size.x; tx++) {
            if (!TilemapCheck(tm, {tx, ty})) {
                continue;
            }

            const t_tile_type_id tile_type_id = tm->types[ty][tx];
            const t_tile_type_info *const tile_type_info = &k_tile_type_infos[tile_type_id];

            const zcl::t_v2 tile_world_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

            SpriteRender(tile_type_info->spr, rendering_context, assets, tile_world_pos);
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


struct t_world {
    zcl::t_b8 paused;
    t_camera camera;
    t_tilemap tilemap;
    t_player player;
};

t_world *WorldCreate(zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_world>(arena);

    TilemapAdd(&result->tilemap, {0, 40}, ek_tile_type_id_dirt);

    return result;
}

void WorldTick(t_world *const world, const zgl::t_input_state *const input_state) {
    if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_escape)) {
        world->paused = !world->paused;
    }

    if (world->paused) {
        return;
    }

    PlayerProcessMovement(&world->player, &world->tilemap, input_state);
    CameraMove(&world->camera, world->player.position);
}

void WorldRender(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state) {
    const auto camera_view_matrix = CameraCreateViewMatrix(world->camera, zgl::BackbufferGetSize(rendering_context.gfx_ticket));
    zgl::RendererPassBegin(rendering_context, zgl::BackbufferGetSize(rendering_context.gfx_ticket), camera_view_matrix, true, k_bg_color);

    TilemapRender(&world->tilemap, rendering_context, assets);

    PlayerRender(&world->player, rendering_context, assets);

    zgl::RendererPassEnd(rendering_context);
}

void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rendering_context, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    if (world->paused) {
        zgl::RendererSubmitRect(rendering_context, zcl::RectCreateF({}, zcl::V2IToF(zgl::BackbufferGetSize(rendering_context.gfx_ticket))), zcl::ColorCreateRGBA32F(0.0f, 0.0f, 0.0f, k_pause_bg_darkness_alpha));
        return;
    }
}
