#include "world_private.h"

#include "camera.h"

namespace world {
    constexpr zcl::t_i32 k_tile_regen_pause_duration = 60;

    // @todo: At some point might want to split between serializable and runtime-relevant state.

    // @note: So currently this represents the high-res tilemap data. What I'm thinking is that once a chunking system is set up, only the tilemap chunks currently active will have this data. All other chunks could be streamed from the world file, or be kept in memory, but either way would just have a bitset for representing activity and tile types (only what's needed).
    struct t_tilemap {
        zcl::t_static_array<zcl::t_static_array<zcl::t_u8, k_tilemap_size.x>, k_tilemap_size.y> lifes;
        zcl::t_static_array<zcl::t_static_array<t_tile_type_id, k_tilemap_size.x>, k_tilemap_size.y> type_ids;
        zcl::t_static_array<zcl::t_static_array<zcl::t_i32, k_tilemap_size.x>, k_tilemap_size.y> regen_pause_times;
    };

    void TilemapUpdate(t_tilemap *const tm) {
        // Most important thing is TilemapUpdate. This is run every frame.
        // We only want to update the tiles that are actually hurt.
        //
        // Idea A:
        //  - Have a bitset for every hurt tile. This is memory efficient and faster than what is currently done.
        //
        // Idea B:
        //  - Have a list for every hurt tile. The list contains the tile positions.
        //      - So it's easy to add a tile when it's hurt. It can be an append or an activity-array type thing.
        //      - But when the tile recovers or is destroyed, what to do?
        //
        // Out of the box:
        // - What if the "life" states weren't even stored per-tile?
        // - An activity array is probably simplest. Set up some arbitrary capacity. Just unset a slot when the tile is destroyed or recovered.
        // - If you use a list (not activity array) then updates are maximally fast (good), you just have slow removal.
        // But when you hurt a tile how do you know where it is
        //
        //
        //
        // Idea:
        // - You can have the "life" state just be discrete in 4 phases (like the anim), and pickaxe strength is instead encoded into the actual item use speed.
        // - Through this, the "TileHurt" calls can be reduced (made periodic), and in these calls you could do a linear lookup of the hurt list to see which one to update, and whether to remove it.
        //      - Per-tick tilemap updates are fast (just iterate through list, no activity check)

        // @speed: REALLY bad!
        for (zcl::t_i32 y = 0; y < k_tilemap_size.y; y++) {
            for (zcl::t_i32 x = 0; x < k_tilemap_size.x; x++) {
                if (!TilemapCheck(tm, {x, y})) {
                    continue;
                }

                if (tm->regen_pause_times[y][x] > 0) {
                    tm->regen_pause_times[y][x]--;
                } else {
                    // @temp
                    const auto tile_type_id = tm->type_ids[y][x];
                    tm->lifes[y][x] = k_tile_types[tile_type_id].life_duration;
                }
            }
        }
    }

    t_tilemap *TilemapCreate(zcl::t_arena *const arena) {
        return zcl::ArenaPush<t_tilemap>(arena);
    }

    zcl::t_b8 TilemapCheckTilePosInBounds(const zcl::t_v2_i pos) {
        return pos.x >= 0 && pos.x < k_tilemap_size.x && pos.y >= 0 && pos.y < k_tilemap_size.y;
    }

    void TilemapAdd(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const t_tile_type_id tile_type) {
        ZCL_ASSERT(TilemapCheckTilePosInBounds(tile_pos));
        ZCL_ASSERT(!TilemapCheck(tm, tile_pos));

        tm->lifes[tile_pos.y][tile_pos.x] = k_tile_types[tile_type].life_duration;
        tm->type_ids[tile_pos.y][tile_pos.x] = tile_type;
    }

    void HurtTile(t_tilemap *const tm, const zcl::t_v2_i tile_pos, const zcl::t_i32 damage, t_item_drop_manager *const item_drop_manager) {
        ZCL_ASSERT(TilemapCheckTilePosInBounds(tile_pos));
        ZCL_ASSERT(TilemapCheck(tm, tile_pos));
        ZCL_ASSERT(damage > 0);

        const auto tile_life = &tm->lifes[tile_pos.y][tile_pos.x];
        const auto tile_type = &k_tile_types[tm->type_ids[tile_pos.y][tile_pos.x]];

        const auto tile_life_last = *tile_life;

        const zcl::t_i32 damage_to_apply = zcl::CalcMin(damage, static_cast<zcl::t_i32>(*tile_life));
        *tile_life -= damage_to_apply;

        if (*tile_life == 0) {
            SpawnItemDrop(item_drop_manager, (zcl::V2IToF(tile_pos) + zcl::t_v2{0.5f, 0.5f}) * k_tile_size, tile_type->drop_item_type_id, 1);
        } else {
            tm->regen_pause_times[tile_pos.y][tile_pos.x] = k_tile_regen_pause_duration;
        }
    }

    zcl::t_b8 TilemapCheck(const t_tilemap *const tm, const zcl::t_v2_i tile_pos) {
        ZCL_ASSERT(TilemapCheckTilePosInBounds(tile_pos));
        return tm->lifes[tile_pos.y][tile_pos.x] > 0;
    }

    zcl::t_rect_i TilemapCalcRectSpan(const zcl::t_rect_f rect) {
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

    void TilemapRender(const t_tilemap *const tm, const zcl::t_rect_i tm_subset, const zgl::t_rendering_context rc, const t_assets *const assets) {
        ZCL_ASSERT(zcl::CheckRectInRect(tm_subset, zcl::RectCreateI(0, 0, k_tilemap_size.x, k_tilemap_size.y)));

        for (zcl::t_i32 ty = zcl::RectGetTop(tm_subset); ty < zcl::RectGetBottom(tm_subset); ty++) {
            for (zcl::t_i32 tx = zcl::RectGetLeft(tm_subset); tx < zcl::RectGetRight(tm_subset); tx++) {
                if (!TilemapCheck(tm, {tx, ty})) {
                    continue;
                }

                const t_tile_type_id tile_type_id = tm->type_ids[ty][tx];
                const t_tile_type *const tile_type = &k_tile_types[tile_type_id];

                const zcl::t_v2 tile_render_pos = zcl::V2IToF(zcl::t_v2_i{tx, ty} * k_tile_size);

                SpriteRender(tile_type->sprite, rc, assets, tile_render_pos);

                const auto tile_life = tm->lifes[ty][tx];
                const auto tile_type_life = k_tile_types[tile_type_id].life_duration;

                if (tile_life < tile_type_life) {
                    const zcl::t_f32 tile_life_perc_inv = 1.0f - (static_cast<zcl::t_f32>(tile_life) / k_tile_types[tile_type_id].life_duration);

                    ZCL_ASSERT(tile_life > 0);
                    const zcl::t_i32 tile_hurt_frame_index = floor(tile_life_perc_inv * 4); // @temp: Once animation system is in place, magic number can be dropped.

                    SpriteRender(static_cast<t_sprite_id>(ek_sprite_id_tile_hurt_0 + tile_hurt_frame_index), rc, assets, tile_render_pos);
                }
            }
        }
    }

    zcl::t_v2_i ScreenToTilePos(const zcl::t_v2 pos_screen, const zcl::t_v2_i screen_size, const t_camera *const camera) {
        const zcl::t_v2 pos_camera = ScreenToCameraPos(pos_screen, screen_size, camera);

        return {
            static_cast<zcl::t_i32>(floor(pos_camera.x / k_tile_size)),
            static_cast<zcl::t_i32>(floor(pos_camera.y / k_tile_size)),
        };
    }
}
