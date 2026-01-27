#include "world.h"

constexpr zcl::t_f32 k_gravity = 0.2f;


// ============================================================
// @section: Tiles and Tilemap
// ============================================================

enum t_tile_type : zcl::t_i8 {
    ek_tile_type_dirt,
    ek_tile_type_stone,
    ek_tile_type_grass,
    ek_tile_type_sand,

    ekm_tile_type_cnt
};

constexpr zcl::t_v2_i k_tilemap_size = {100, 100};

struct t_tilemap {
    zcl::t_static_bitset<k_tilemap_size.x * k_tilemap_size.y> activity;
    zcl::t_static_array<zcl::t_static_array<t_tile_type, k_tilemap_size.x>, k_tilemap_size.y> types;
};

static zcl::t_b8 TilePosCheckInBounds(const zcl::t_v2_i pos) {
    return pos.x >= 0 && pos.x < k_tilemap_size.x && pos.y >= 0 && pos.y < k_tilemap_size.y;
}

static void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type tile_type) {
    ZCL_ASSERT(TilePosCheckInBounds(tile_pos));

    zcl::BitsetSet(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
    tm->types[tile_pos.y][tile_pos.x] = tile_type;
}

static void TilemapRemove(t_tilemap *const tm, const zcl::t_v2_i tile_pos) {
    ZCL_ASSERT(TilePosCheckInBounds(tile_pos));
    zcl::BitsetUnset(tm->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
}

// ============================================================


// ============================================================
// @section: Player
// ============================================================

constexpr zcl::t_f32 k_player_move_spd = 1.5f;
constexpr zcl::t_f32 k_player_move_spd_acc = 0.2f;
constexpr zcl::t_f32 k_player_jump_height = 3.5f;

struct t_player {
    zcl::t_v2 pos;
    zcl::t_v2 vel;
    zcl::t_b8 jumping;
};

static bool PlayerCheckGrounded(const zcl::t_v2 player_pos, const t_tilemap *const tilemap) {
    // const s_rect below_collider = RectTranslated(PlayerCollider(player_pos), (s_v2){0.0f, 1.0f});
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

// ============================================================


struct t_world {
    t_tilemap tilemap;
    t_player player;
};

t_world *WorldCreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_world>(arena);
}

void WorldTick(t_world *const world, const zgl::t_input_state *const input_state) {
    PlayerProcessMovement(&world->player, &world->tilemap, input_state);
}

void WorldRender(t_world *const world) {
}
