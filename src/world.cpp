#include "world_private.h"

#include "tiles.h"
#include "camera.h"

namespace world {
    t_world *WorldCreate(const zgl::t_gfx_ticket_mut gfx_ticket, zcl::t_arena *const arena) {
        const auto result = zcl::ArenaPush<t_world>(arena);

        result->rng = zcl::RNGCreate(zcl::RandGenSeed(), arena);

        result->tilemap = WorldGen(result->rng, arena);

        result->player_meta = CreatePlayerMeta(arena);

        const zcl::t_v2 world_size = zcl::V2IToF(k_tilemap_size * k_tile_size);
        result->player_entity = CreatePlayerEntity(result->player_meta, result->tilemap, arena);

        result->npc_manager = CreateNPCManager(arena);

        result->camera = CameraCreate(GetPlayerPos(result->player_entity), 2.0f, 0.3f, arena);

        result->ui = UICreate(arena);

        SpawnNPC(result->npc_manager, {k_tile_size * k_tilemap_size.x * 0.5f, 0.0f}, ek_npc_type_id_slime);

        return result;
    }

    static void ProcessPlayerAndNPCCollisions(const t_player_entity *const player_entity, const t_npc_manager *const npc_manager) {
        const zcl::t_rect_f player_collider = GetPlayerCollider(player_entity->pos);

        ZCL_BITSET_WALK_ALL_SET (npc_manager->activity, i) {
            const auto npc = &npc_manager->buf[i];
            const zcl::t_rect_f npc_collider = GetNPCCollider(npc->pos, npc->type_id);

            if (zcl::CheckInters(player_collider, npc_collider)) {
                zcl::Log(ZCL_STR_LITERAL("ashdjklasd"));
            }
        }
    }

    t_world_tick_result_id WorldTick(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena) {
        t_world_tick_result_id result_id = ek_world_tick_result_id_normal;

        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        UIProcessPlayerInventoryInteraction(world->ui, GetPlayerInventory(world->player_meta), input_state);

        ProcessPlayerInventoryHotbarUpdates(world->player_meta, input_state);

#if 0
        if (zgl::KeyCheckPressed(input_state, zgl::ek_key_code_x)) {
            const auto pop_up = PopUpSpawn(&world->pop_ups, world->player_entity.pos, {0.0f, -6.0f});

            // @todo: This is pretty annoying to do, maybe in ZF there could be better abstractions for this?
            zcl::t_byte_stream str_bytes_stream = zcl::ByteStreamCreate(pop_up->str_bytes, zcl::ek_stream_mode_write);
            zcl::Print(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("YEAH"));
            pop_up->str_byte_cnt = zcl::ByteStreamGetWritten(&str_bytes_stream).len;
        }
#endif

        ProcessPlayerMovement(world->player_entity, world->tilemap, input_state);

        ProcessPlayerItemUsage(world->player_meta, world->player_entity, world->tilemap, assets, input_state, screen_size, temp_arena);

        ProcessNPCAIs(world->npc_manager, world->tilemap);

        ProcessPlayerAndNPCCollisions(world->player_entity, world->npc_manager);

        ProcessNPCDeaths(world->npc_manager);

        CameraMove(world->camera, GetPlayerPos(world->player_entity));

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

        RenderPlayer(world->player_entity, rc, assets);

        RenderNPCs(world->npc_manager, rc, assets);

        zgl::RendererPassEnd(rc);
    }

    zcl::t_str_mut DetermineCursorHoverStr(const zcl::t_v2 cursor_pos, const t_npc_manager *const npc_manager, const t_camera *const camera, const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
        constexpr zcl::t_i32 k_str_len_limit = 32;

        ZCL_BITSET_WALK_ALL_SET (npc_manager->activity, i) {
            const auto npc = &npc_manager->buf[i];
            const zcl::t_rect_f npc_collider = GetNPCCollider(npc->pos, npc->type_id);
            const zcl::t_rect_f npc_collider_screen = zcl::RectCreateF(CameraToScreenPos(zcl::RectGetPos(npc_collider), camera, screen_size), zcl::RectGetSize(npc_collider) * CameraGetScale(camera));

            if (zcl::CheckPointInRect(cursor_pos, npc_collider_screen)) {
                return zcl::StrClone(g_npc_types[npc->type_id].name, arena);
            }
        }

        return {};
    }

    void WorldRenderUI(const t_world *const world, const zgl::t_rendering_context rc, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
        const zcl::t_v2 cursor_pos = zgl::CursorGetPos(input_state);

        UIRenderPopUps(rc, &world->pop_ups, world->camera, assets, temp_arena);
        UIRenderTileHighlight(rc, cursor_pos, world->camera);
        UIRenderCursorHoverStr(rc, cursor_pos, world->npc_manager, world->camera, assets, temp_arena);
        UIRenderPlayerInventory(world->ui, rc, GetPlayerInventory(world->player_meta), assets, temp_arena);
        UIRenderPlayerHealth(rc);
        UIRenderCursorHeldItem(world->ui, rc, cursor_pos, assets, temp_arena);
    }
}
