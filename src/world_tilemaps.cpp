#include "world_private.h"

#include "camera.h"

namespace world {

#if 0
    // Okay so there's a world file which needs to be split into chunks.
    // The world file can contain a chunk size inside it (in case its an old version or something)
    // From that chunk size you can index into the chunk of interest and load the data in.
    //
    // But how do we know what chunk we're in at at runtime?
    //
    // Don't go infinite.
    //
    // Idea 1:
    // We can just have a 2D array of chunk pointers, and a 2D bitset indicating which are active.
    // Then on demand a chunk can lazily be loaded from the file.
    // This is maximally flexible and works with the current tilemap public API in which you can do whatever you want
    // with whatever tile you want.
    // But it ignores the runtime reality that really only chunks near the screen are of interest.
    //
    // --------------------
    //
    // Runtime tilemap and serialized/file tilemap are very different things!
    // - Because chunking is totally irrelevant when doing world gen for example.
    //
    //
    //
    // Actually might need to have the entire low-res tilemap in memory when doing world gen, otherwise it gets super messy.
    //
    //
    // What's the simplest approach?
    // - From the public API you specify a "simulation region", this can basically be the player view
    // - From the public API you have to manually specify which things are brought into memory. This way you know when calling
    //
    // We've intertwined 2 distinct systems!

    constexpr zcl::t_v2_i k_tilemap_chunk_size = {200, 40};
    static_assert(k_tilemap_size.x % k_tilemap_chunk_size.x == 0 && k_tilemap_size.y % k_tilemap_chunk_size.y == 0);

    constexpr zcl::t_i32 k_tilemap_tile_regen_pause_duration = 60;

    struct t_tilemap_res_high {
        zcl::t_array_mut<zcl::t_array_mut<zcl::t_i32>> lifes;
        zcl::t_array_mut<zcl::t_array_mut<zcl::t_i32>> regen_pause_times;
    };

    struct t_tilemap {
        t_tilemap_res_high s;

        zcl::t_static_bitset<k_tilemap_size.x * k_tilemap_size.y> activity;
        zcl::t_static_array<zcl::t_static_array<t_tile_type_id, k_tilemap_size.x>, k_tilemap_size.y> type_ids;
        zcl::t_static_array<zcl::t_static_array<t_tilemap_res_high *, k_tilemap_size.x / k_tilemap_chunk_size.x>, k_tilemap_size.y / k_tilemap_chunk_size.y> chunks;
    };

    t_tilemap *TilemapCreate(zcl::t_arena *const arena) {
        return zcl::ArenaPush<t_tilemap>(arena);
    }

    void TilemapUpdate(t_tilemap *const tilemap, zcl::t_arena *const temp_arena) {
    #if 0
        const auto tm_indexes = zcl::BitsetLoadIndexesOfSet(tilemap->activity, temp_arena);

        for (zcl::t_i32 i = 0; i < tm_indexes.len; i++) {
            const zcl::t_i32 yeah = tm_indexes[i];
            const zcl::t_i32 x = yeah % k_tilemap_size.x;
            const zcl::t_i32 y = yeah / k_tilemap_size.x;

            if (tilemap->regen_pause_times[y][x] > 0) {
                tilemap->regen_pause_times[y][x]--;
            } else {
                // @temp
                const auto tile_type_id = tilemap->type_ids[y][x];
                tilemap->lifes[y][x] = k_tile_types[tile_type_id].life_duration;
            }
        }
    #endif
    }

    zcl::t_b8 TilemapCheckTilePosInBounds(const zcl::t_v2_i pos) {
        return pos.x >= 0 && pos.x < k_tilemap_size.x && pos.y >= 0 && pos.y < k_tilemap_size.y;
    }

    void TilemapAdd(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
        ZCL_ASSERT(TilemapCheckTilePosInBounds(tile_pos));
        ZCL_ASSERT(!TilemapCheck(tilemap, tile_pos));

    #if 0
        tilemap->lifes[tile_pos.y][tile_pos.x] = k_tile_types[tile_type].life_duration;
    #endif
        zcl::BitsetSet(tilemap->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
        tilemap->type_ids[tile_pos.y][tile_pos.x] = tile_type;
    }

    void TilemapHurt(t_tilemap *const tilemap, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage, t_item_drop_manager *const item_drop_manager) {
        ZCL_ASSERT(TilemapCheckTilePosInBounds(tile_pos));
        ZCL_ASSERT(TilemapCheck(tilemap, tile_pos));
        ZCL_ASSERT(damage > 0);

        // @todo: Could lazily bring chunks in?

    #if 0
        const auto tile_life = &tilemap->lifes[tile_pos.y][tile_pos.x];
        const auto tile_type = &k_tile_types[tilemap->type_ids[tile_pos.y][tile_pos.x]];

        const auto tile_life_last = *tile_life;

        const zcl::t_i32 damage_to_apply = zcl::CalcMin(damage, static_cast<zcl::t_i32>(*tile_life));
        *tile_life -= damage_to_apply;

        if (*tile_life == 0) {
            zcl::BitsetUnset(tilemap->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.x);
            SpawnItemDrop(item_drop_manager, (zcl::V2IToF(tile_pos) + zcl::t_v2{0.5f, 0.5f}) * k_tile_size, tile_type->drop_item_type_id, 1);
        } else {
            tilemap->regen_pause_times[tile_pos.y][tile_pos.x] = k_tilemap_tile_regen_pause_duration;
        }
    #endif
    }

    zcl::t_b8 TilemapCheck(const t_tilemap *const tilemap, const zcl::t_v2_i tile_pos) {
        ZCL_ASSERT(TilemapCheckTilePosInBounds(tile_pos));
        return zcl::BitsetCheckSet(tilemap->activity, (tile_pos.y * k_tilemap_size.x) + tile_pos.y);
    }

    zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect) {
        const zcl::t_i32 left = static_cast<zcl::t_i32>(zcl::Floor(rect.x / k_tile_size));
        const zcl::t_i32 top = static_cast<zcl::t_i32>(zcl::Floor(rect.y / k_tile_size));
        const zcl::t_i32 right = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetRight(rect) / k_tile_size));
        const zcl::t_i32 bottom = static_cast<zcl::t_i32>(zcl::Ceil(zcl::RectGetBottom(rect) / k_tile_size));

        const zcl::t_rect_i result_without_clamp = {
            left,
            top,
            right - left,
            bottom - top,
        };

        return zcl::ClampWithinContainer(result_without_clamp, zcl::RectCreateI({}, k_tilemap_size));
    }

    zcl::t_b8 TilemapCheckCollision(const t_tilemap *const tilemap, const zcl::t_rect_f collider) {
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

    // @todo: Move to world!
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

    // @todo: Move to world!
    zcl::t_v2 TilemapMoveContact(const zcl::t_v2 pos_current, const zcl::t_cardinal_direction_id cardinal_dir_id, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
        zcl::t_v2 pos_next = pos_current;

        // Jump by tile intervals first, then make more precise contact.
        pos_next = TilemapMoveContactByJumpSize(pos_next, k_tile_size, cardinal_dir_id, collider_size, collider_origin, tilemap);
        pos_next = TilemapMoveContactByJumpSize(pos_next, k_tilemap_move_contact_jump_size_precise, cardinal_dir_id, collider_size, collider_origin, tilemap);

        return pos_next;
    }

    // @todo: Move to world!
    static void TilemapProcessCollisionsVertical(zcl::t_v2 *const pos, zcl::t_f32 *const vel_y, const zcl::t_v2 collider_size, const zcl::t_v2 collider_origin, const t_tilemap *const tilemap) {
        const zcl::t_rect_f collider_vertical = ColliderCreate({pos->x, pos->y + *vel_y}, collider_size, collider_origin);

        if (TilemapCheckCollision(tilemap, collider_vertical)) {
            *pos = TilemapMoveContactByJumpSize(*pos, k_tilemap_move_contact_jump_size_precise, *vel_y >= 0.0f ? zcl::ek_cardinal_direction_down : zcl::ek_cardinal_direction_up, collider_size, collider_origin, tilemap);
            *vel_y = 0.0f;
        }
    }

    // @todo: Specific technique for collision handling should probably not be in here...
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

    void TilemapRender(const t_tilemap *const tilemap, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets) {
        ZCL_ASSERT(zcl::CheckRectInRect(tm_subset, zcl::RectCreateI(0, 0, k_tilemap_size.x, k_tilemap_size.y)));

        for (zcl::t_i32 ty = zcl::RectGetTop(tm_subset); ty < zcl::RectGetBottom(tm_subset); ty++) {
            for (zcl::t_i32 tx = zcl::RectGetLeft(tm_subset); tx < zcl::RectGetRight(tm_subset); tx++) {
                if (!TilemapCheck(tilemap, {tx, ty})) {
                    continue;
                }

                const t_tile_type_id tile_type_id = tilemap->type_ids[ty][tx];
                const t_tile_type *const tile_type = &k_tile_types[tile_type_id];

                const zcl::t_v2 tile_render_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

                SpriteRender(tile_type->sprite, rc, assets, tile_render_pos);

    #if 0
                const auto tile_life = tilemap->lifes[ty][tx];
                const auto tile_type_life = k_tile_types[tile_type_id].life_duration;

                if (tile_life < tile_type_life) {
                    const zcl::t_f32 tile_life_perc_inv = 1.0f - (static_cast<zcl::t_f32>(tile_life) / k_tile_types[tile_type_id].life_duration);

                    ZCL_ASSERT(tile_life > 0);
                    const zcl::t_i32 tile_hurt_frame_index = zcl::Floor(tile_life_perc_inv * 4); // @temp: Once animation system is in place, magic number can be dropped.

                    SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_tile_hurt_0 + tile_hurt_frame_index), rc, assets, tile_render_pos);
                }
    #endif
            }
        }
    }

    zcl::t_v2_i ScreenToTilePos(const zcl::t_v2 pos_screen, const zcl::t_v2_i screen_size, const t_camera *const camera) {
        const zcl::t_v2 pos_camera = ScreenToCameraPos(pos_screen, screen_size, camera);

        return {
            static_cast<zcl::t_i32>(zcl::Floor(pos_camera.x / k_tile_size)),
            static_cast<zcl::t_i32>(zcl::Floor(pos_camera.y / k_tile_size)),
        };
    }
#endif
}
