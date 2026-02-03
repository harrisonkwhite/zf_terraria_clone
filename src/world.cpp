#include "world_private.h"

#include "tiles.h"
#include "camera.h"

namespace world {
    constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

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

        result->tilemap = WorldGen(result->rng, arena);

        result->player_meta = PlayerCreateMeta(arena);

        const zcl::t_v2 world_size = zcl::V2IToF(k_tilemap_size * k_tile_size);
        result->player_entity = PlayerCreateEntity(result->player_meta, {}, arena);
        // result->player_entity.pos = MakeContactWithTilemap({world_size.x * 0.5f, 0.0f}, zcl::ek_cardinal_direction_down, zcl::RectGetSize(PlayerEntityColliderCreate(result->player_entity.pos)), k_player_entity_origin, result->tilemap);

        result->camera = CameraCreate(PlayerGetPos(result->player_entity), 2.0f, 0.3f, arena);

        result->ui = UICreate(arena);

        return result;
    }

    t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
        t_world_tick_result_id result_id = ek_world_tick_result_id_normal;

        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        UIPlayerInventoryProcessInteraction(world->ui, PlayerGetInventory(world->player_meta), input_state);

        PlayerProcessInventoryHotbarUpdate(world->player_meta, input_state);

#if 0
        if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_x)) {
            const auto pop_up = PopUpSpawn(&world->pop_ups, world->player_entity.pos, {0.0f, -6.0f});

            // @todo: This is pretty annoying to do, maybe in ZF there could be better abstractions for this?
            zcl::t_byte_stream str_bytes_stream = zcl::ByteStreamCreate(pop_up->str_bytes, zcl::ek_stream_mode_write);
            zcl::Print(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("YEAH"));
            pop_up->str_byte_cnt = zcl::ByteStreamGetWritten(&str_bytes_stream).len;
        }
#endif

        PlayerProcessMovement(world->player_entity, world->tilemap, input_state);

        PlayerProcessItemUsage(world->player_meta, world->player_entity, world->tilemap, assets, input_state, screen_size, temp_arena);

        CameraMove(world->camera, PlayerGetPos(world->player_entity));

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

    static zcl::t_rect_i CameraCalcTilemapRect(const t_camera *const camera, const zcl::t_v2_i screen_size) {
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

        TilemapRender(world->tilemap, CameraCalcTilemapRect(world->camera, rc.screen_size), rc, assets);

        PlayerRender(world->player_entity, rc, assets);

        zgl::RendererPassEnd(rc);
    }

    void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        UIRenderPopUps(rc, &world->pop_ups, world->camera, assets, temp_arena);
        UIRenderTileHighlight(rc, cursor_pos, world->camera);
        UIRenderPlayerInventory(world->ui, rc, PlayerGetInventory(world->player_meta), assets, temp_arena);
        UIRenderPlayerHealth(rc);
        UIRenderCursorHeldItem(world->ui, rc, cursor_pos, assets, temp_arena);
    }
}
