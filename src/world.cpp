// @todo: A cleanup of this stinky file.
// @todo: World UI logic should be in its own file. Both render and tick.

#include "world_private.h"

#include "sprites.h"

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


struct t_world {
    zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?
    t_camera camera;
    t_tilemap tilemap;
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

t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i window_framebuffer_size, zcl::t_arena *const temp_arena) {
    t_world_tick_result_id result_id = ek_world_tick_result_id_normal;

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
