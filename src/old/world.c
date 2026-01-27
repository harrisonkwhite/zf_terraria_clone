#include "game.h"

#include <stdio.h>
#include "lighting.h"

#define RESPAWN_TIME 120

static s_matrix_4x4 CameraViewMatrix(const s_camera* const cam, const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 view_pos = {
        (-cam->pos.x * cam->scale) + (window_size.x / 2.0f),
        (-cam->pos.y * cam->scale) + (window_size.y / 2.0f)
    };

    s_matrix_4x4 mat = IdentityMatrix4x4();
    TranslateMatrix4x4(&mat, view_pos);
    ScaleMatrix4x4(&mat, cam->scale);
    return mat;
}

static inline t_r32 CalcCameraScale(const s_v2_s32 window_size) {
    return window_size.x > 1600 || window_size.y > 900 ? 3.0f : 2.0f;
}

bool InitWorld(s_world* const world, const t_world_filename* const filename, const s_v2_s32 window_size, s_mem_arena* const temp_mem_arena) {
    assert(world && IS_ZERO(*world));
    assert(filename);

    if (!InitMemArena(&world->mem_arena, WORLD_MEM_ARENA_SIZE)) {
        return false;
    }

    if (!LoadWorldCoreFromFile(&world->core, filename)) {
        return false;
    }

    InitPlayer(&world->player, world->core.player_hp_max, &world->core.tilemap_core.activity);

    world->cam = (s_camera){
        .pos = world->player.pos,
        .scale = CalcCameraScale(window_size)
    };

    AddToInventory((s_inventory_slot*)world->player_inv_slots, PLAYER_INVENTORY_LEN, ek_item_type_copper_pickaxe, 1);

    return true;
}

void CleanWorld(s_world* const world) {
    CleanMemArena(&world->mem_arena);
}

bool WorldTick(s_world* const world, const t_settings* const settings, const s_game_tick_context* const zfw_context) {
    world->cam.scale = CalcCameraScale(zfw_context->window_state.size);
    const s_v2 cam_size = CameraSize(world->cam.scale, zfw_context->window_state.size);

    //world->biome = DetermineWorldBiome(world, zfw_context->window_state.size);

    if (!world->player.killed) {
        if (!UpdatePlayer(world, &zfw_context->input_context)) {
            return false;
        }
    }

    if (world->player.killed) {
        if (world->respawn_time < RESPAWN_TIME) {
            world->respawn_time++;
        } else {
            world->respawn_time = 0;

            ZERO_OUT(world->player);
            InitPlayer(&world->player, world->core.player_hp_max, &world->core.tilemap_core.activity);
        }
    }

    if (!ProcEnemySpawning(world, cam_size.x)) {
        return false;
    }

    UpdateNPCs(world);
    ProcNPCDeaths(world); // NOTE: Might need to defer this until later in the tick.

    if (!UpdateItemDrops(world, settings)) {
        return false;
    }

    if (!ProcItemUsage(world, &zfw_context->input_context, zfw_context->window_state.size)) {
        return false;
    }

    if (!UpdateProjectiles(world)) {
        return false;
    }

    UpdateWorldUI(world, &zfw_context->input_context, zfw_context->window_state.size);

    // Update the camera.
    {
        const s_v2 cam_pos_dest = world->player.pos;

        if (SettingToggle(settings, ek_setting_smooth_camera)) {
            world->cam.pos.x = Lerp(world->cam.pos.x, cam_pos_dest.x, CAMERA_LERP);
            world->cam.pos.y = Lerp(world->cam.pos.y, cam_pos_dest.y, CAMERA_LERP);

            world->cam.pos = (s_v2){
                CLAMP(world->cam.pos.x, cam_size.x / 2.0f, WORLD_WIDTH - (cam_size.x / 2.0f)),
                CLAMP(world->cam.pos.y, cam_size.y / 2.0f, WORLD_HEIGHT - (cam_size.y / 2.0f))
            };
        } else {
            world->cam.pos = cam_pos_dest;
        }
    }

    UpdateParticles(&world->particles, GRAVITY);

    return true;
}

static s_rect_edges_s32 TilemapRenderRange(const s_camera* const cam, const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 cam_tl = CameraTopLeft(cam, window_size);
    const s_v2 cam_size = CameraSize(cam->scale, window_size);

    s_rect_edges_s32 render_range = {
        .left = floorf(cam_tl.x / TILE_SIZE),
        .top = floorf(cam_tl.y / TILE_SIZE),
        .right = ceilf((cam_tl.x + cam_size.x) / TILE_SIZE),
        .bottom = ceilf((cam_tl.y + cam_size.y) / TILE_SIZE)
    };

    // Clamp the tilemap render range within tilemap bounds.
    render_range.left = CLAMP(render_range.left, 0, TILEMAP_WIDTH - 1);
    render_range.top = CLAMP(render_range.top, 0, TILEMAP_HEIGHT - 1);
    render_range.right = CLAMP(render_range.right, 0, TILEMAP_WIDTH);
    render_range.bottom = CLAMP(render_range.bottom, 0, TILEMAP_HEIGHT);

    return render_range;
}

static bool WARN_UNUSED_RESULT GenWorldLightmap(s_lightmap* const lightmap, s_mem_arena* const mem_arena, const t_tilemap_activity* const tm_activity, const s_rect_edges_s32 tm_render_range, s_mem_arena* const temp_mem_arena) {
    assert(IS_ZERO(*lightmap));

    if (!GenLightmap(lightmap, mem_arena, (s_v2_s32){tm_render_range.right - tm_render_range.left, tm_render_range.bottom - tm_render_range.top})) {
        return false;
    }

    for (t_s32 ty = tm_render_range.top; ty < tm_render_range.bottom; ty++) {
        for (t_s32 tx = tm_render_range.left; tx < tm_render_range.right; tx++) {
            if (IsTileActive(tm_activity, (s_v2_s32){tx, ty})) {
                continue;
            }

            const s_v2_s32 lp = {tx - tm_render_range.left, ty - tm_render_range.top};
            SetLightLevel(*lightmap, lp, LIGHT_LEVEL_LIMIT);
        }
    }

    return PropagateLights(*lightmap, temp_mem_arena);
}

bool RenderWorld(const s_world* const world, const s_rendering_context* const rendering_context, const s_texture_group* const textures, const s_surface* const temp_surf, s_mem_arena* const temp_mem_arena) {
    const s_matrix_4x4 cam_view_mat = CameraViewMatrix(&world->cam, rendering_context->window_size);
    SetViewMatrix(rendering_context, &cam_view_mat);

    const s_rect_edges_s32 tilemap_render_range = TilemapRenderRange(&world->cam, rendering_context->window_size);

    RenderTilemap(&world->core.tilemap_core, rendering_context, &world->tilemap_tile_lifes, tilemap_render_range, textures);

    if (!world->player.killed) {
        RenderPlayer(&world->player, rendering_context, textures, temp_surf);
    }

    RenderNPCs(&world->npcs, rendering_context, textures);

    RenderItemDrops(world->item_drops, world->item_drop_active_cnt, rendering_context, textures);

    RenderProjectiles((s_projectile_array_view){.buf_raw = world->projectiles, .elem_cnt = world->proj_cnt}, rendering_context, textures);

    RenderParticles(&world->particles, rendering_context, textures);

    s_lightmap world_lightmap = {0};

    if (!GenWorldLightmap(&world_lightmap, temp_mem_arena, &world->core.tilemap_core.activity, tilemap_render_range, temp_mem_arena)) {
        return false;
    }

    RenderLightmap(rendering_context, world_lightmap, (s_v2){tilemap_render_range.left * TILE_SIZE, tilemap_render_range.top * TILE_SIZE}, TILE_SIZE);

    return true;
}

bool LoadWorldCoreFromFile(s_world_core* const world_core, const t_world_filename* const filename) {
    assert(world_core && IS_ZERO(*world_core));
    assert(filename);

    FILE* const fs = fopen((const char*)filename, "rb");

    if (!fs) {
        return false;
    }

    if (fread(world_core, sizeof(*world_core), 1, fs) == 0) {
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool WriteWorldCoreToFile(const s_world_core* const world_core, const t_world_filename* const filename) {
    assert(world_core);
    assert(filename);

    FILE* const fs = fopen((const char*)filename, "wb");

    if (!fs) {
        LOG_ERROR("Failed to open \"%s\"!", (const char*)filename);
        return false;
    }

    if (fwrite(world_core, sizeof(*world_core), 1, fs) == 0) {
        LOG_ERROR("Failed to write to world file \"%s\"!", (const char*)filename);
        fclose(fs);
        return false;
    }

    fclose(fs);

    return true;
}

bool PlaceWorldTile(s_world* const world, const s_v2_s32 pos, const e_tile_type type) {
    AddTile(&world->core.tilemap_core, pos, type);
    world->tilemap_tile_lifes[pos.y][pos.x] = 0;
    return true;
}

bool HurtWorldTile(s_world* const world, const s_v2_s32 pos) {
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    world->tilemap_tile_lifes[pos.y][pos.x]++;

    const s_tile_type_info* const tile_type = &g_tile_type_infos[world->core.tilemap_core.tile_types[pos.y][pos.x]];

    {
        // Spawn particles.
        const s_v2 tile_mid = {
            (pos.x + 0.5f) * TILE_SIZE,
            (pos.y + 0.5f) * TILE_SIZE
        };

        const t_s32 part_cnt = RandRangeS32(3, 5);

        for (t_s32 i = 0; i < part_cnt; i++) {
            const s_v2 vel = {
                RandRangeIncl(-0.5f, 0.5f),
                RandRangeIncl(-2.5f, -1.0f)
            };

            SpawnParticleFromTemplate(&world->particles, tile_type->particle_template, tile_mid, vel, TAU * RandPerc());
        }
    }

    if (world->tilemap_tile_lifes[pos.y][pos.x] == tile_type->life) {
        if (!DestroyWorldTile(world, pos)) {
            return false;
        }
    }

    return true;
}

bool DestroyWorldTile(s_world* const world, const s_v2_s32 pos) {
assert(world);
    assert(IsTilePosInBounds(pos));
    assert(IsTileActive(&world->core.tilemap_core.activity, pos));

    RemoveTile(&world->core.tilemap_core, pos);

    // Spawn item drop.
    const s_v2 drop_pos = {(pos.x + 0.5f) * TILE_SIZE, (pos.y + 0.5f) * TILE_SIZE};
    const s_tile_type_info* const tile_type = &g_tile_type_infos[world->core.tilemap_core.tile_types[pos.y][pos.x]];

    if (!SpawnItemDrop(world, drop_pos, tile_type->drop_item, 1)) {
        return false;
    }

    return true;
}

bool IsTilePosFree(const s_world* const world, const s_v2_s32 tile_pos) {
    assert(world);
    assert(IsTilePosInBounds(tile_pos));
    assert(!IsTileActive(&world->core.tilemap_core.activity, tile_pos));

    const s_rect tile_collider = {
        tile_pos.x * TILE_SIZE,
        tile_pos.y * TILE_SIZE,
        TILE_SIZE,
        TILE_SIZE
    };

    // Check for player.
    const s_rect player_collider = PlayerCollider(world->player.pos);

    if (DoRectsInters(tile_collider, player_collider)) {
        return false;
    }

    // Check for NPCs.
    for (t_s32 i = 0; i < NPC_LIMIT; i++) {
        if (!IsNPCActivityBitSet(&world->npcs.activity, i)) {
            continue;
        }

        const s_npc* const npc = &world->npcs.buf[i];
        const s_rect npc_collider = NPCCollider(npc->pos, npc->type);

        if (DoRectsInters(tile_collider, npc_collider)) {
            return false;
        }
    }

    // Check for projectiles.
    for (t_s32 i = 0; i < world->proj_cnt; i++) {
        const s_projectile* const proj = &world->projectiles[i];
        const s_rect proj_collider = ProjectileCollider(proj->type, proj->pos);

        if (DoRectsInters(tile_collider, proj_collider)) {
            return false;
        }
    }

    return true;
}

e_world_biome DetermineWorldBiome(const s_world* const world, const s_v2_s32 screen_size) {
    const s_v2 cam_size = CameraSize(world->cam.scale, screen_size);

    const int tile_range_margin = 32;

    const s_rect_edges_s32 tile_range_edges = {
        .left = floorf(world->cam.pos.x / TILE_SIZE) - tile_range_margin,
        .top = floorf(world->cam.pos.y / TILE_SIZE) - tile_range_margin,
        .right = ceilf((world->cam.pos.x + cam_size.x) / TILE_SIZE) + tile_range_margin,
        .bottom = ceilf((world->cam.pos.y + cam_size.y) / TILE_SIZE) + tile_range_margin
    };

    const int sand_cnt = TileTypeCnt(&world->core.tilemap_core, tile_range_edges, ek_tile_type_sand); // NOTE: This is not ideal. Ideally we want to sample the tile type counts that we're interested in within a single pass. Maybe we could have an array of the tile types we want to cache, or an enum of them.

    if (sand_cnt > 64) {
        return ek_world_biome_desert;
    }

    return ek_world_biome_overworld;
}

s_popup_text* SpawnPopupText(s_world* const world, const s_v2 pos, const t_r32 vel_y) {
    for (t_s32 i = 0; i < POPUP_TEXT_LIMIT; i++) {
        s_popup_text* const popup = &world->popup_texts[i];

        if (popup->alpha > POPUP_TEXT_INACTIVITY_ALPHA_THRESH) {
            continue;
        }

        ZERO_OUT(*popup);

        popup->pos = pos;
        popup->alpha = 1.0f;
        popup->vel_y = vel_y;

        return popup;
    }

    LOG_ERROR("Failed to spawn popup text due to insufficient space!");

    return NULL;
}
