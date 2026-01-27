#include "game.h"

#define TILEMAP_CONTACT_PRECISE_JUMP_SIZE 0.1f

const s_tile_type_info g_tile_type_infos[] = {
    [ek_tile_type_dirt] = {
        .spr = ek_sprite_dirt_tile,
        .drop_item = ek_item_type_dirt_block,
        .life = 5,
        .particle_template = ek_particle_template_dirt
    },
    [ek_tile_type_stone] = {
        .spr = ek_sprite_stone_tile,
        .drop_item = ek_item_type_stone_block,
        .life = 8,
        .particle_template = ek_particle_template_stone
    },
    [ek_tile_type_grass] = {
        .spr = ek_sprite_grass_tile,
        .drop_item = ek_item_type_grass_block,
        .life = 3,
        .particle_template = ek_particle_template_grass
    },
    [ek_tile_type_sand] = {
        .spr = ek_sprite_sand_tile,
        .drop_item = ek_item_type_sand_block,
        .life = 3,
        .particle_template = ek_particle_template_sand
    }
};

STATIC_ARRAY_LEN_CHECK(g_tile_type_infos, eks_tile_type_cnt);

void AddTile(s_tilemap_core* const tm_core, const s_v2_s32 pos, const e_tile_type tile_type) {
    assert(IsTilePosInBounds(pos));
    assert(!IsTileActive(&tm_core->activity, pos));

    SetTilemapActivityBit(&tm_core->activity, IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH));
    tm_core->tile_types[pos.y][pos.x] = tile_type;
}

void RemoveTile(s_tilemap_core* const tm_core, const s_v2_s32 pos) {
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&tm_core->activity, pos));

    UnsetTilemapActivityBit(&tm_core->activity, IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH));
}

s_rect_edges_s32 RectTilemapSpan(const s_rect rect) {
    return RectEdgesS32Clamped(
        (s_rect_edges_s32){
            rect.x / TILE_SIZE,
            rect.y / TILE_SIZE,
            ceilf((rect.x + rect.width) / TILE_SIZE),
            ceilf((rect.y + rect.height) / TILE_SIZE)
        },
        (s_rect_edges_s32){0, 0, TILEMAP_WIDTH, TILEMAP_HEIGHT}
    );
}

bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider) {
    const s_rect_edges_s32 collider_tilemap_span = RectTilemapSpan(collider);

    for (t_s32 ty = collider_tilemap_span.top; ty < collider_tilemap_span.bottom; ty++) {
        for (t_s32 tx = collider_tilemap_span.left; tx < collider_tilemap_span.right; tx++) {
            if (!IsTileActive(tm_activity, (s_v2_s32){tx, ty})) {
                continue;
            }

            const s_rect tile_collider = {
                TILE_SIZE * tx,
                TILE_SIZE * ty,
                TILE_SIZE,
                TILE_SIZE
            };

            if (DoRectsInters(collider, tile_collider)) {
                return true;
            }
        }
    }

    return false;
}

void ProcTileCollisions(s_v2* const pos, s_v2* const vel, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    const s_rect hor_collider = Collider((s_v2){pos->x + vel->x, pos->y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, hor_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, vel->x >= 0.0f ? ek_cardinal_dir_right : ek_cardinal_dir_left, collider_size, collider_origin, tm_activity);
        vel->x = 0.0f;
    }

    ProcVerTileCollisions(pos, &vel->y, collider_size, collider_origin, tm_activity);

    const s_rect diag_collider = Collider(V2Sum(*pos, *vel), collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, diag_collider)) {
        vel->x = 0.0f;
    }
}

void ProcVerTileCollisions(s_v2* const pos, t_r32* const vel_y, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    const s_rect ver_collider = Collider((s_v2){pos->x, pos->y + *vel_y}, collider_size, collider_origin);

    if (TileCollisionCheck(tm_activity, ver_collider)) {
        MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, *vel_y >= 0.0f ? ek_cardinal_dir_down : ek_cardinal_dir_up, collider_size, collider_origin, tm_activity);
        *vel_y = 0.0f;
    }
}

void MakeContactWithTilemap(s_v2* const pos, const e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    // Jump by tile intervals first, then make more precise contact.
    MakeContactWithTilemapByJumpSize(pos, TILE_SIZE, dir, collider_size, collider_origin, tm_activity);
    MakeContactWithTilemapByJumpSize(pos, TILEMAP_CONTACT_PRECISE_JUMP_SIZE, dir, collider_size, collider_origin, tm_activity);
}

void MakeContactWithTilemapByJumpSize(s_v2* const pos, const t_r32 jump_size, const e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity) {
    assert(jump_size > 0.0f);

    const s_v2_s32 jump_dir = g_cardinal_dirs[dir];
    const s_v2 jump = {jump_dir.x * jump_size, jump_dir.y * jump_size};

    while (!TileCollisionCheck(tm_activity, Collider(V2Sum(*pos, jump), collider_size, collider_origin))) {
        *pos = V2Sum(*pos, jump);
    }
}

void RenderTilemap(const s_tilemap_core* const tilemap_core, const s_rendering_context* const rendering_context, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const s_rect_edges_s32 range, const s_texture_group* const textures) {
    assert(IsTilemapRangeValid(range));

    for (t_s32 ty = range.top; ty < range.bottom; ty++) {
        for (t_s32 tx = range.left; tx < range.right; tx++) {
            if (!IsTileActive(&tilemap_core->activity, (s_v2_s32){tx, ty})) {
                continue;
            }

            const e_tile_type tile_type = *STATIC_ARRAY_2D_ELEM(tilemap_core->tile_types, ty, tx);
            const s_tile_type_info* const tile_type_info = STATIC_ARRAY_ELEM(g_tile_type_infos, tile_type);
            const s_v2 tile_world_pos = {tx * TILE_SIZE, ty * TILE_SIZE};
            RenderSprite(rendering_context, tile_type_info->spr, textures, tile_world_pos, (s_v2){0}, (s_v2){1.0f, 1.0f}, 0.0f, WHITE);

            // Render the break overlay.
            const t_s32 tile_life = *STATIC_ARRAY_2D_ELEM(*tilemap_tile_lifes, ty, tx);

            if (tile_life > 0) {
                const t_s32 tile_life_max = tile_type_info->life;
                const t_s32 break_spr_cnt = 4; // TODO: This is really bad. We need an animation frame system of some kind.
                const t_r32 break_index_mult = (t_r32)tile_life / tile_life_max;
                const t_s32 break_index = break_spr_cnt * break_index_mult;
                assert(tile_life < tile_life_max); // Sanity check.

                RenderSprite(rendering_context, ek_sprite_tile_break_0 + break_index, textures, tile_world_pos, (s_v2){0}, (s_v2){1.0f, 1.0f}, 0.0f, WHITE);
            }
        }
    }
}
