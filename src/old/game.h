#ifndef GAME_H
#define GAME_H

#include <zfwc.h>
#include "sprites.h"
#include "particles.h"

#define GAME_TITLE "Terraria"

#define BG_COLOR (u_v4){0.25f, 0.42f, 0.61f, 1.0f}

#define DEATH_TEXT "You were slain..."

#define TILE_SIZE 8
#define TILEMAP_WIDTH 640
#define TILEMAP_HEIGHT 128
#define WORLD_WIDTH (TILE_SIZE * TILEMAP_WIDTH)
#define WORLD_HEIGHT (TILE_SIZE * TILEMAP_HEIGHT)

#define GRAVITY 0.2f
#define CAMERA_LERP 0.3f

#define TILE_PLACE_DIST 4
#define TILE_HIGHLIGHT_ALPHA 0.4f

#define PLAYER_INIT_HP_MAX 100
#define PLAYER_ORIGIN (s_v2){0.5f, 0.5f}
#define PLAYER_HURT_FLASH_TIME 10

#define ITEM_DROP_ORIGIN (s_v2){0.5f, 0.5f}

#define POPUP_TEXT_LIMIT 1024
#define POPUP_TEXT_STR_BUF_SIZE 32
#define POPUP_TEXT_INACTIVITY_ALPHA_THRESH 0.001f
#define POPUP_TEXT_VEL_Y_MULT 0.9f
#define POPUP_TEXT_FADE_VEL_Y_ABS_THRESH 0.002f
#define POPUP_TEXT_ALPHA_MULT 0.9f

#define DMG_POPUP_TEXT_VEL_Y_MIN -4.0f
#define DMG_POPUP_TEXT_VEL_Y_MAX -2.5f
static_assert(DMG_POPUP_TEXT_VEL_Y_MIN <= DMG_POPUP_TEXT_VEL_Y_MAX, "Invalid range.");

#define MOUSE_HOVER_STR_BUF_SIZE 32
typedef char t_mouse_hover_str_buf[MOUSE_HOVER_STR_BUF_SIZE];

#define PLAYER_INVENTORY_COLUMN_CNT 6
#define PLAYER_INVENTORY_ROW_CNT 3
#define PLAYER_INVENTORY_LEN (PLAYER_INVENTORY_COLUMN_CNT * PLAYER_INVENTORY_ROW_CNT)
static_assert(PLAYER_INVENTORY_COLUMN_CNT <= 9, "Too large since each hotbar slot needs an associated digit key.");
#define TILE_PLACE_DEFAULT_USE_BREAK 2

#define NPC_LIMIT 256
#define NPC_ORIGIN (s_v2){0.5f, 0.5f}
#define NPC_HURT_FLASH_TIME 10

#define PROJECTILE_LIMIT 1024
#define ITEM_DROP_LIMIT 1024
#define WORLD_MEM_ARENA_SIZE ((1 << 20) * 2)

#define WORLD_NAME_LEN_LIMIT 20
#define WORLD_FILENAME_EXT ".wrld"
#define WORLD_FILENAME_BUF_SIZE (WORLD_NAME_LEN_LIMIT + sizeof(WORLD_FILENAME_EXT))
#define WORLD_LIMIT 3
typedef char t_world_filename[WORLD_FILENAME_BUF_SIZE];
typedef t_world_filename t_world_filenames[WORLD_LIMIT];
typedef char t_world_name_buf[WORLD_NAME_LEN_LIMIT + 1];

#define SETTINGS_FILENAME "settings.dat"
#define ITEM_QUANTITY_LIMIT 99 // TEMP: Eventually each item type will have its own unique quantity.

typedef enum {
    ek_tile_type_dirt,
    ek_tile_type_stone,
    ek_tile_type_grass,
    ek_tile_type_sand,
    eks_tile_type_cnt
} e_tile_type;

typedef enum {
    ek_item_type_dirt_block,
    ek_item_type_stone_block,
    ek_item_type_grass_block,
    ek_item_type_sand_block,
    ek_item_type_copper_pickaxe,
    ek_item_type_wooden_sword,
    ek_item_type_wooden_bow,
    eks_item_type_cnt
} e_item_type;

typedef struct {
    e_item_type item_type;
    t_s32 quantity;
} s_inventory_slot;

typedef enum {
    ek_item_use_type_tile_place,
    ek_item_use_type_tile_hurt,
    ek_item_use_type_shoot
} e_item_use_type;

typedef enum {
    ek_projectile_type_wooden_arrow,
    eks_projectile_type_cnt
} e_projectile_type;

typedef enum {
    ek_projectile_type_flags_falls = 1 << 0,
    ek_projectile_type_flags_rot_is_dir = 1 << 1
} e_projectile_type_flags;

typedef enum {
    ek_npc_type_slime,
    eks_npc_type_cnt
} e_npc_type;

typedef enum {
    ek_title_screen_tick_result_type_normal,
    ek_title_screen_tick_result_type_error,
    ek_title_screen_tick_result_type_load_world,
    ek_title_screen_tick_result_type_exit
} e_title_screen_tick_result_type;

typedef enum {
    ek_title_screen_page_home,
    ek_title_screen_page_worlds,
    ek_title_screen_page_new_world,
    ek_title_screen_page_settings,
    eks_title_screen_page_cnt
} e_title_screen_page;

typedef enum {
    ek_setting_type_toggle,
    ek_setting_type_perc
} e_setting_type;

typedef enum {
    ek_setting_smooth_camera,
    ek_setting_volume,
    eks_setting_cnt
} e_setting;

typedef struct {
    e_sprite spr;
    e_item_type drop_item;
    t_s32 life;
    e_particle_template particle_template;
} s_tile_type_info;

extern const s_tile_type_info g_tile_type_infos[];

typedef struct {
    s_char_array_view name;

    e_sprite icon_spr;

    bool consume_on_use;
    e_item_use_type use_type;
    t_s32 use_break;

    e_tile_type tile_place_type;

    t_s32 tile_hurt_dist;

    e_projectile_type shoot_proj_type;
    t_r32 shoot_proj_spd;
    t_s32 shoot_proj_dmg;
} s_item_type_info;

extern const s_item_type_info g_item_type_infos[];

typedef struct {
    e_sprite spr;
    e_projectile_type_flags flags;
} s_projectile_type_info;

extern const s_projectile_type_info g_projectile_type_infos[];

typedef struct {
    bool killed;
    s_v2 pos;
    s_v2 vel;
    bool jumping;
    t_s32 hp;
    t_s32 invinc_time;
    t_s32 item_use_break;
    t_s32 flash_time;
} s_player;

typedef struct {
    t_s32 jump_time;
    t_r32 jump_hor_spd;
} s_slime_npc;

typedef union {
    s_slime_npc slime;
} u_npc_type_data;

typedef struct {
    s_v2 pos;
    s_v2 vel;
    t_s32 hp;
    t_s32 flash_time;
    e_npc_type type;
    u_npc_type_data type_data;
} s_npc;

DEF_STATIC_BITSET_TYPE(npc_activity, NPCActivity, NPC_LIMIT);

typedef struct {
    s_npc buf[NPC_LIMIT];
    t_npc_activity activity;
} s_npcs;

typedef struct {
    e_projectile_type type;
    bool friendly;
    t_s32 dmg;
    s_v2 pos;
    s_v2 vel;
    t_r32 rot;
} s_projectile;

DEF_ARRAY_TYPE(s_projectile, projectile, Projectile);

typedef struct {
    e_item_type item_type;
    t_s32 quantity;
    s_v2 pos;
    s_v2 vel;
} s_item_drop;

typedef struct {
    char str[POPUP_TEXT_STR_BUF_SIZE];
    s_v2 pos;
    t_r32 vel_y;
    t_r32 alpha;
} s_popup_text;

typedef struct {
    s_v2 pos;
    t_r32 scale;
} s_camera;

DEF_STATIC_BITSET_TYPE(tilemap_activity, TilemapActivity, TILEMAP_WIDTH * TILEMAP_HEIGHT);

typedef struct {
    t_tilemap_activity activity;
    e_tile_type tile_types[TILEMAP_HEIGHT][TILEMAP_WIDTH];
} s_tilemap_core;

typedef t_s32 t_tilemap_tile_lifes[TILEMAP_HEIGHT][TILEMAP_WIDTH];

typedef struct {
    t_s32 player_hp_max;
    s_tilemap_core tilemap_core;
} s_world_core;

typedef enum {
    ek_world_biome_overworld,
    ek_world_biome_desert,
    ek_world_biome_ocean
} e_world_biome;

static int TileTypeCnt(const s_tilemap_core* const tm_core, const s_rect_edges_s32 tm_range, const e_tile_type tile_type) {
    //assert(IsTilemapRangeValid(tm_range));

    int cnt = 0;

    for (int ty = 0; ty < TILEMAP_HEIGHT; ty++) {
        for (int tx = 0; tx < TILEMAP_WIDTH; tx++) {
            const e_tile_type tt = *STATIC_ARRAY_2D_ELEM(tm_core->tile_types, ty, tx);

            if (tt == tile_type) {
                cnt++;
            }
        }
    }

    return cnt;
}

typedef struct world {
    s_mem_arena mem_arena;

    s_world_core core;

    t_s32 respawn_time;

    e_world_biome biome;

    s_player player;

    s_npcs npcs;

    s_projectile projectiles[PROJECTILE_LIMIT];
    t_s32 proj_cnt;

    s_item_drop item_drops[ITEM_DROP_LIMIT];
    t_s32 item_drop_active_cnt;

    t_tilemap_tile_lifes tilemap_tile_lifes;

    s_particles particles;

    s_camera cam;

    bool player_inv_open;
    s_inventory_slot player_inv_slots[PLAYER_INVENTORY_ROW_CNT][PLAYER_INVENTORY_COLUMN_CNT];
    t_s32 player_inv_hotbar_slot_selected;

    s_popup_text popup_texts[POPUP_TEXT_LIMIT];

    t_mouse_hover_str_buf mouse_hover_str;
    e_item_type mouse_item_held_type;
    t_s32 mouse_item_held_quantity;
} s_world;

typedef void (*t_npc_tick_func)(s_world* const world, const t_s32 npc_index);
typedef void (*t_npc_postspawn_func)(s_world* const world, const t_s32 npc_index);

typedef struct {
    s_char_array_view name;
    e_sprite spr;
    t_npc_tick_func tick_func;
    t_npc_postspawn_func postspawn_func;
    t_s32 hp_max;
    t_s32 contact_dmg;
    t_r32 contact_kb;
} s_npc_type;

typedef struct {
    e_setting_type type;
    s_char_array_view name;
    t_u8 preset;
} s_setting;

typedef t_u8 t_settings[eks_setting_cnt];
extern const s_setting g_settings[];

typedef struct {
    e_title_screen_tick_result_type type;
    t_world_filename world_filename;
} s_title_screen_tick_result;

typedef struct {
    e_title_screen_page page;
    t_s32 page_btn_feat_hovered_index;
    t_world_filenames world_filenames_cache;
    t_world_name_buf new_world_name_buf;
} s_title_screen;

typedef struct {
    s_texture_group textures;
    s_font_group fonts;
    s_shader_prog_group shader_progs;

    s_surface global_surf;
    s_surface temp_surf;

    t_settings settings;

    bool in_world;
    s_title_screen title_screen;
    s_world world;
} s_game;

static inline t_r32 UIScale(const s_v2_s32 window_size) {
    if (window_size.x > 1920 && window_size.y > 1080) {
        return 2.0f;
    }

    if (window_size.x > 1600 && window_size.y > 900) {
        return 1.5f;
    }

    return 1.0f;
}

static inline s_v2_s32 UISize(const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);
    const t_r32 ui_scale = UIScale(window_size);
    return (s_v2_s32){window_size.x / ui_scale, window_size.y / ui_scale};
}

static inline s_v2 DisplayToUIPos(const s_v2 pos, const s_v2_s32 window_size) {
    const t_r32 ui_scale = UIScale(window_size);
    return (s_v2){pos.x / ui_scale, pos.y / ui_scale};
}

static inline s_rect Collider(const s_v2 pos, const s_v2 size, const s_v2 origin) {
    assert(size.x > 0.0f && size.y > 0.0f);
    return (s_rect){pos.x - (size.x * origin.x), pos.y - (size.y * origin.y), size.x, size.y};
}

static inline s_rect ColliderFromSprite(const e_sprite spr, const s_v2 pos, const s_v2 origin) {
    return Collider(pos, (s_v2){g_sprites[spr].src_rect.width, g_sprites[spr].src_rect.height}, origin);
}

static inline bool SettingToggle(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_toggle);
    return (*settings)[setting];
}

static inline t_r32 SettingPerc(const t_settings* const settings, const e_setting setting) {
    assert(g_settings[setting].type == ek_setting_type_perc);
    return (t_r32)(*settings)[setting] / 100.0f;
}

static inline s_v2_s32 CameraToTilePos(const s_v2 pos) {
    return (s_v2_s32){
        floorf(pos.x / TILE_SIZE),
        floorf(pos.y / TILE_SIZE)
    };
}

static inline s_v2 CameraSize(const t_r32 cam_scale, const s_v2_s32 window_size) {
    assert(cam_scale > 0.0f);
    assert(window_size.x > 0 && window_size.y > 0);
    return (s_v2){window_size.x / cam_scale, window_size.y / cam_scale};
}

static inline s_v2 CameraTopLeft(const s_camera* const cam, const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 size = CameraSize(cam->scale, window_size);
    return (s_v2){cam->pos.x - (size.x / 2.0f), cam->pos.y - (size.y / 2.0f)};
}

static inline s_v2 CameraToDisplayPos(const s_v2 pos, const s_camera* const cam, const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 cam_tl = CameraTopLeft(cam, window_size);
    return (s_v2) {
        (pos.x - cam_tl.x) * cam->scale,
        (pos.y - cam_tl.y) * cam->scale
    };
}

static inline s_v2 DisplayToCameraPos(const s_v2 pos, const s_camera* const cam, const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);

    const s_v2 cam_tl = CameraTopLeft(cam, window_size);
    return (s_v2) {
        cam_tl.x + (pos.x / cam->scale),
        cam_tl.y + (pos.y / cam->scale)
    };
}

static inline s_v2 CameraToUIPos(const s_v2 pos, const s_camera* const cam, const s_v2_s32 window_size) {
    assert(window_size.x > 0 && window_size.y > 0);
    return DisplayToUIPos(CameraToDisplayPos(pos, cam, window_size), window_size);
}

//
// game.c
//
bool InitGame(const s_game_init_context* const context);
e_game_tick_result GameTick(const s_game_tick_context* const context);
bool RenderGame(const s_game_render_context* const context);
void CleanGame(void* const dev_mem);

//
// title_screen.c
//
bool InitTitleScreen(s_title_screen* const ts, s_mem_arena* const temp_mem_arena);
s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, t_settings* const settings, const s_game_tick_context* const zfw_context, const s_font_group* const fonts);
bool RenderTitleScreen(const s_title_screen* const ts, const s_rendering_context* const rendering_context, const t_settings* const settings, const s_texture_group* const textures, const s_font_group* const fonts, s_mem_arena* const temp_mem_arena);

//
// world.c
//
bool InitWorld(s_world* const world, const t_world_filename* const filename, const s_v2_s32 window_size, s_mem_arena* const temp_mem_arena);
void CleanWorld(s_world* const world);
bool WorldTick(s_world* const world, const t_settings* const settings, const s_game_tick_context* const zfw_context);
bool RenderWorld(const s_world* const world, const s_rendering_context* const rendering_context, const s_texture_group* const textures, const s_surface* const temp_surf, s_mem_arena* const temp_mem_arena);
bool LoadWorldCoreFromFile(s_world_core* const world_core, const t_world_filename* const filename);
bool WriteWorldCoreToFile(const s_world_core* const world_core, const t_world_filename* const filename);
bool PlaceWorldTile(s_world* const world, const s_v2_s32 pos, const e_tile_type type);
bool HurtWorldTile(s_world* const world, const s_v2_s32 pos);
bool DestroyWorldTile(s_world* const world, const s_v2_s32 pos);
bool IsTilePosFree(const s_world* const world, const s_v2_s32 tile_pos);
e_world_biome DetermineWorldBiome(const s_world* const world, const s_v2_s32 screen_size);
s_popup_text* SpawnPopupText(s_world* const world, const s_v2 pos, const t_r32 vel_y);

//
// world_ui.c
//
void UpdateWorldUI(s_world* const world, const s_input_context* const input_context, const s_v2_s32 window_size);
bool RenderWorldUI(const s_world* const world, const s_game_render_context* const context, const s_texture_group* const textures, const s_font_group* const fonts);

//
// world_gen.c
//
void GenWorld(s_world_core* const world_core);

//
// player.c
//
void InitPlayer(s_player* const player, const t_s32 hp_max, const t_tilemap_activity* const tm_activity);
bool UpdatePlayer(s_world* const world, const s_input_context* const input_context);
void RenderPlayer(const s_player* const player, const s_rendering_context* const rendering_context, const s_texture_group* const textures, const s_surface* const temp_surf);
bool HurtPlayer(s_world* const world, const t_s32 dmg, const s_v2 kb);

static inline s_v2 PlayerColliderSize() {
    const s_sprite* const spr = &g_sprites[ek_sprite_player];
    return (s_v2){spr->src_rect.width, spr->src_rect.height};
}

static inline s_rect PlayerCollider(const s_v2 pos) {
    return Collider(pos, PlayerColliderSize(), PLAYER_ORIGIN);
}

//
// npcs.c
//
extern const s_npc_type g_npc_types[];

t_s32 SpawnNPC(s_world* const world, const s_v2 pos, const e_npc_type type, const t_tilemap_activity* const tm_activity);
void UpdateNPCs(s_world* const world);
void ProcNPCDeaths(s_world* const world);
void RenderNPCs(const s_npcs* const npcs, const s_rendering_context* const rendering_context, const s_texture_group* const textures);
bool HurtNPC(s_world* const world, const t_s32 npc_index, const t_s32 dmg, const s_v2 kb);
bool ProcEnemySpawning(s_world* const world, const t_r32 cam_width);

static inline s_v2 NPCColliderSize(const e_npc_type npc_type) {
    const s_sprite* const spr = &g_sprites[g_npc_types[npc_type].spr];
    return (s_v2){spr->src_rect.width, spr->src_rect.height};
}

static inline s_rect NPCCollider(const s_v2 npc_pos, const e_npc_type npc_type) {
    return Collider(npc_pos, NPCColliderSize(npc_type), NPC_ORIGIN);
}

//
// projectiles.c
//
s_projectile* SpawnProjectile(s_world* const world, const e_projectile_type type, const bool friendly, const t_s32 dmg, const s_v2 pos, const s_v2 vel);
bool UpdateProjectiles(s_world* const world);
void RenderProjectiles(const s_projectile_array_view projectiles, const s_rendering_context* const rendering_context, const s_texture_group* const textures);

static inline s_rect ProjectileCollider(const e_projectile_type proj_type, const s_v2 pos) {
    return ColliderFromSprite(g_projectile_type_infos[proj_type].spr, pos, (s_v2){0.5f, 0.5f});
}

//
// items.c
//
bool IsItemUsable(const e_item_type item_type, const s_world* const world, const s_v2_s32 mouse_tile_pos);
bool ProcItemUsage(s_world* const world, const s_input_context* const input_context, const s_v2_s32 window_size);
bool SpawnItemDrop(s_world* const world, const s_v2 pos, const e_item_type item_type, const t_s32 item_quantity);
bool UpdateItemDrops(s_world* const world, const t_settings* const settings);
void RenderItemDrops(const s_item_drop* const drops, const t_s32 drop_cnt, const s_rendering_context* const rendering_context, const s_texture_group* const textures);

static inline s_v2 ItemDropColliderSize(const e_item_type item_type) {
    const s_sprite* const spr = &g_sprites[g_item_type_infos[item_type].icon_spr];
    return (s_v2){spr->src_rect.width, spr->src_rect.height};
}

static inline s_rect ItemDropCollider(const s_v2 pos, const e_item_type item_type) {
    return Collider(pos, ItemDropColliderSize(item_type), ITEM_DROP_ORIGIN);
}

//
// tilemap.c
//
void AddTile(s_tilemap_core* const tm_core, const s_v2_s32 pos, const e_tile_type tile_type);
void RemoveTile(s_tilemap_core* const tm_core, const s_v2_s32 pos);
s_rect_edges_s32 RectTilemapSpan(const s_rect rect);
bool TileCollisionCheck(const t_tilemap_activity* const tm_activity, const s_rect collider);
void ProcTileCollisions(s_v2* const pos, s_v2* const vel, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void ProcVerTileCollisions(s_v2* const pos, t_r32* const vel_y, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemap(s_v2* const pos, const e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void MakeContactWithTilemapByJumpSize(s_v2* const pos, const t_r32 jump_size, const e_cardinal_dir dir, const s_v2 collider_size, const s_v2 collider_origin, const t_tilemap_activity* const tm_activity);
void RenderTilemap(const s_tilemap_core* const tilemap_core, const s_rendering_context* const rendering_context, const t_tilemap_tile_lifes* const tilemap_tile_lifes, const s_rect_edges_s32 range, const s_texture_group* const textures);

static inline bool IsTilePosInBounds(const s_v2_s32 pos) {
    return pos.x >= 0 && pos.x < TILEMAP_WIDTH && pos.y >= 0 && pos.y < TILEMAP_HEIGHT;
}

static inline bool IsTilemapRangeValid(const s_rect_edges_s32 range) {
    return IsRangeS32Valid(range, (s_v2_s32){TILEMAP_WIDTH, TILEMAP_HEIGHT});
}

static inline t_s32 TileDist(const s_v2_s32 a, const s_v2_s32 b) {
    return Dist((s_v2){a.x, a.y}, (s_v2){b.x, b.y});
}

static bool IsTileActive(const t_tilemap_activity* const tm_activity, const s_v2_s32 pos) {
    assert(IsTilePosInBounds(pos));
    return IsTilemapActivityBitSet(tm_activity, IndexFrom2D(pos.x, pos.y, TILEMAP_WIDTH));
}

//
// inventory.c
//
t_s32 AddToInventory(s_inventory_slot* const slots, const t_s32 slot_cnt, const e_item_type item_type, t_s32 quantity);
t_s32 RemoveFromInventory(s_inventory_slot* const slots, const t_s32 slot_cnt, const e_item_type item_type, t_s32 quantity);
bool DoesInventoryHaveRoomFor(s_inventory_slot* const slots, const t_s32 slot_cnt, const e_item_type item_type, t_s32 quantity);

#endif
