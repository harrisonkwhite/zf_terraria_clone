#include "world.h"

#include "sprites.h"

constexpr zcl::t_f32 k_gravity = 0.2f;


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
    ek_tile_type_id_sand,

    ekm_tile_type_id_cnt
};

constexpr zcl::t_static_array<t_tile_type_info, ekm_tile_type_id_cnt> k_tile_type_infos = {{
    {.spr = ek_sprite_id_dirt_tile},
    {.spr = ek_sprite_id_stone_tile},
    {.spr = ek_sprite_id_grass_tile},
    {.spr = ek_sprite_id_sand_tile},
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
    const zcl::t_rect_i result_without_clamp = {
        static_cast<zcl::t_i32>(zcl::RectGetLeft(rect) / k_tile_size),
        static_cast<zcl::t_i32>(zcl::RectGetTop(rect) / k_tile_size),
        static_cast<zcl::t_i32>(ceil(zcl::RectGetRight(rect) / k_tile_size)) - k_tilemap_size.x,
        static_cast<zcl::t_i32>(ceil(zcl::RectGetBottom(rect) / k_tile_size)) - k_tilemap_size.y,
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

#if 0
static void ProcTileCollisions(zcl::t_v2 *const pos, zcl::t_v2 *const vel, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f hor_collider = Collider((zcl::t_v2){pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, hor_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, vel->x >= 0.0f ? ek_cardinal_dir_right : ek_cardinal_dir_left, collider_size, collider_origin, tm_activity);
        vel->x = 0.0f;
    }

    ProcVerTileCollisions(pos, &vel->y, collider_size, collider_origin, tm_activity);

    const zcl::t_rect_f diag_collider = Collider(*pos + *vel, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, diag_collider)) {
        vel->x = 0.0f;
    }
}

static void ProcVerTileCollisions(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    const zcl::t_rect_f ver_collider = Collider((zcl::t_v2){pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TileCollisionCheck(tilemap, ver_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, *vel_y >= 0.0f ? ek_cardinal_dir_down : ek_cardinal_dir_up, collider_size, collider_origin, tm_activity);
        *vel_y = 0.0f;
    }
}

static void MakeContactWithTilemap(zcl::t_v2 *const pos, const e_cardinal_dir dir, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap_activity *const tm_activity) {
    // Jump by tile intervals first, then make more precise contact.
    MakeContactWithTilemapByJumpSize(pos, TILE_SIZE, dir, collider_size, collider_origin, tm_activity);
    MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, dir, collider_size, collider_origin, tm_activity);
}

static void MakeContactWithTilemapByJumpSize(zcl::t_v2 *const pos, const zcl::t_f32 jump_size, const e_cardinal_dir dir, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
    ZCL_ASSERT(jump_size > 0.0f);

    const s_v2_s32 jump_dir = g_cardinal_dirs[dir];
    const zcl::t_v2 jump = {jump_dir.x * jump_size, jump_dir.y * jump_size};

    while (!TileCollisionCheck(tilemap, Collider(V2Sum(*pos, jump), collider_size, collider_origin))) {
        *pos = V2Sum(*pos, jump);
    }
}
#endif

#if 0
void MakeContactWithTilemap(zcl::t_v2 *const pos, const e_cardinal_dir dir, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap_activity *const tm_activity) {
    // Jump by tile intervals first, then make more precise contact.
    MakeContactWithTilemapByJumpSize(pos, TILE_SIZE, dir, collider_size, collider_origin, tm_activity);
    MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, dir, collider_size, collider_origin, tm_activity);
}

void MakeContactWithTilemapByJumpSize(zcl::t_v2 *const pos, const t_r32 jump_size, const e_cardinal_dir dir, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap_activity *const tm_activity) {
    assert(jump_size > 0.0f);

    const s_v2_s32 jump_dir = g_cardinal_dirs[dir];
    const zcl::t_v2 jump = {jump_dir.x * jump_size, jump_dir.y * jump_size};

    while (!TileCollisionCheck(tm_activity, Collider(V2Sum(*pos, jump), collider_size, collider_origin))) {
        *pos = V2Sum(*pos, jump);
    }
}

void RenderTilemap(const s_tilemap_core *const tilemap_core, const s_rendering_context *const rendering_context, const t_tilemap_tile_lifes *const tilemap_tile_lifes, const s_rect_edges_s32 range, const s_texture_group *const textures) {
    assert(IsTilemapRangeValid(range));

    for (t_s32 ty = range.top; ty < range.bottom; ty++) {
        for (t_s32 tx = range.left; tx < range.right; tx++) {
            if (!IsTileActive(&tilemap_core->activity, (s_v2_s32){tx, ty})) {
                continue;
            }

            const e_tile_type tile_type = *STATIC_ARRAY_2D_ELEM(tilemap_core->tile_types, ty, tx);
            const s_tile_type_info *const tile_type_info = STATIC_ARRAY_ELEM(g_tile_type_infos, tile_type);
            const zcl::t_v2 tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(rendering_context, tile_type_info->spr, textures, tile_world_pos, (zcl::t_v2){0}, (zcl::t_v2){1.0f, 1.0f}, 0.0f, WHITE);

            // Render the break overlay.
            const t_s32 tile_life = *STATIC_ARRAY_2D_ELEM(*tilemap_tile_lifes, ty, tx);

            if (tile_life > 0) {
                const t_s32 tile_life_max = tile_type_info->life;
                const t_s32 break_spr_cnt = 4; // TODO: This is really bad. We need an animation frame system of some kind.
                const t_r32 break_index_mult = (t_r32)tile_life / tile_life_max;
                const t_s32 break_index = break_spr_cnt * break_index_mult;
                assert(tile_life < tile_life_max); // Sanity check.

                RenderSprite(rendering_context, ek_sprite_tile_break_0 + break_index, textures, tile_world_pos, (zcl::t_v2){0}, (zcl::t_v2){1.0f, 1.0f}, 0.0f, WHITE);
            }
        }
    }
}
#endif

void TilemapRender(const t_tilemap *const tm, const zgl::t_rendering_context rc, const t_assets *const assets) {
    for (zcl::t_i32 ty = 0; ty < k_tilemap_size.y; ty++) {
        for (zcl::t_i32 tx = 0; tx < k_tilemap_size.x; tx++) {
            if (!TilemapCheck(tm, {tx, ty})) {
                continue;
            }

            const t_tile_type_id tile_type_id = tm->types[ty][tx];
            const t_tile_type_info *const tile_type_info = &k_tile_type_infos[tile_type_id];

            const zcl::t_v2 tile_world_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

            RendererSubmitSprite(rc, tile_type_info->spr, assets, tile_world_pos);
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
    zcl::t_v2 pos;
    zcl::t_v2 vel;
    zcl::t_b8 jumping;
};

static zcl::t_b8 PlayerCheckGrounded(const zcl::t_v2 player_pos, const t_tilemap *const tilemap) {
    // const s_rect below_collider = RectTranslated(PlayerCollider(player_pos), (zcl::t_v2){0.0f, 1.0f});
    // return TileCollisionCheck(tm_activity, below_collider);
    return false;
}

static void PlayerProcessMovement(t_player *const player, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state) {
    const zcl::t_f32 move_axis = zgl::KeyCheckDown(input_state, zgl::ek_key_code_d) - zgl::KeyCheckDown(input_state, zgl::ek_key_code_a);

    const zcl::t_f32 move_spd_dest = move_axis * k_player_move_spd;

    if (player->vel.x < move_spd_dest) {
        player->vel.x += zcl::CalcMin(move_spd_dest - player->vel.x, k_player_move_spd_acc);
    } else if (player->vel.x > move_spd_dest) {
        player->vel.x -= zcl::CalcMin(player->vel.x - move_spd_dest, k_player_move_spd_acc);
    }

    player->vel.y += k_gravity;

    const zcl::t_b8 grounded = PlayerCheckGrounded(player->pos, tilemap);

    if (grounded) {
        player->jumping = false;
    }

    if (!player->jumping) {
        if (grounded && zgl::KeyCheckPressed(input_state, zgl::ek_key_code_space)) {
            player->vel.y = -k_player_jump_height;
            player->jumping = true;
        }
    } else {
        if (player->vel.y < 0.0f && !zgl::KeyCheckDown(input_state, zgl::ek_key_code_space)) {
            player->vel.y = 0.0f;
        }
    }

    // ProcTileCollisions(&player->pos, &player->vel, PlayerColliderSize(), PLAYER_ORIGIN, &world->core.tilemap_core.activity);

    player->pos += player->vel;
}

static void PlayerRender(const t_player *const player, const zgl::t_rendering_context rc, const t_assets *const assets) {
    RendererSubmitSprite(rc, ek_sprite_id_player, assets, player->pos, k_player_origin);
}

// ============================================================


struct t_world {
    t_tilemap tilemap;
    t_player player;
};

t_world *WorldCreate(zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_world>(arena);

    TilemapAdd(&result->tilemap, {}, ek_tile_type_id_dirt);

    return result;
}

void WorldTick(t_world *const world, const zgl::t_input_state *const input_state) {
    PlayerProcessMovement(&world->player, &world->tilemap, input_state);
}

void WorldRender(t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets) {
    zgl::RendererPassBegin(rc, zgl::BackbufferGetSize(rc.gfx_ticket));

    TilemapRender(&world->tilemap, rc, assets);

    PlayerRender(&world->player, rc, assets);

    zgl::RendererPassEnd(rc);
}
