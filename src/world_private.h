#pragma once

#include "world_public.h"
#include "npcs.h"
#include "items.h"
#include "tiles.h"

// ============================================================
// @section: External Forward Declarations

struct t_camera;
struct t_inventory;
struct t_tilemap;

// ==================================================

namespace world {
    // ============================================================
    // @section: Types and Constants

    constexpr zcl::t_color_rgba32f k_bg_color = zcl::ColorCreateRGBA32F(0.35f, 0.77f, 1.0f);

    constexpr zcl::t_f32 k_gravity = 0.2f;

    constexpr zcl::t_i32 k_player_respawn_duration = 120;
    constexpr zcl::t_i32 k_player_invincible_duration = 30;
    constexpr zcl::t_f32 k_player_move_spd = 1.5f;
    constexpr zcl::t_f32 k_player_move_spd_acc = 0.2f;
    constexpr zcl::t_f32 k_player_jump_height = 3.5f;
    constexpr zcl::t_v2 k_player_origin = zcl::k_origin_center;
    constexpr zcl::t_i32 k_player_flash_duration = 10;

    constexpr zcl::t_i32 k_npc_limit = 1024;
    constexpr zcl::t_v2 k_npc_origin = zcl::k_origin_center;
    constexpr zcl::t_i32 k_npc_flash_duration = 10;

    constexpr zcl::t_i32 k_item_drop_limit = 1024;
    constexpr zcl::t_f32 k_item_drop_item_type_icon_scale = 0.5f;
    constexpr zcl::t_v2 k_item_drop_origin = {0.5f, 0.5f};

    constexpr zcl::t_i32 k_pop_up_limit = 1024;
    constexpr zcl::t_i32 k_pop_up_death_duration = 15;
    constexpr zcl::t_f32 k_pop_up_lerp_factor = 0.15f;

    constexpr zcl::t_f32 k_ui_tile_highlight_alpha = 0.6f;
    constexpr zcl::t_v2 k_ui_player_health_bar_offs_top_right = {48.0f, 48.0f};
    constexpr zcl::t_v2 k_ui_player_health_bar_size = {240.0f, 24.0f};
    constexpr zcl::t_f32 k_ui_player_health_bar_bg_alpha = 0.4f;
    constexpr zcl::t_v2 k_ui_player_inventory_offs_top_left = {48.0f, 48.0f};
    constexpr zcl::t_f32 k_ui_player_inventory_slot_size = 48.0f;
    constexpr zcl::t_f32 k_ui_player_inventory_slot_distance = 64.0f;
    constexpr zcl::t_f32 k_ui_player_inventory_slot_bg_alpha = 0.4f;

    struct t_player_meta {
        zcl::t_i32 respawn_time;

        zcl::t_i32 health_limit;

        t_inventory *inventory;
        zcl::t_i32 inventory_hotbar_slot_selected_index;
    };

    struct t_player_entity {
        zcl::t_b8 active;

        zcl::t_i32 health;
        zcl::t_i32 invincible_time;

        zcl::t_i32 item_use_time;

        zcl::t_v2 pos;
        zcl::t_v2 vel;
        zcl::t_b8 jumping;

        zcl::t_i32 flash_time; // @note: Could be derived from invincible_time?
    };

    struct t_npc {
        zcl::t_v2 pos;

        zcl::t_i32 health;

        zcl::t_i32 flash_time;

        t_npc_type_id type_id;

        union {
            struct {
                zcl::t_v2 vel;
            } slime;
        } type_data;
    };

    struct t_npc_manager {
        zcl::t_static_array<t_npc, k_npc_limit> buf;
        zcl::t_static_bitset<k_npc_limit> activity;
        zcl::t_static_array<zcl::t_i32, k_npc_limit> versions;
    };

    struct t_npc_id {
        zcl::t_i32 index;
        zcl::t_i32 version;
    };

    struct t_item_drop {
        zcl::t_v2 pos;
        zcl::t_v2 vel;

        t_item_type_id item_type_id;
    };

    struct t_item_drop_manager {
        zcl::t_static_array<t_item_drop, k_item_drop_limit> buf;
        zcl::t_static_bitset<k_item_drop_limit> activity;
    };

    struct t_pop_up {
        zcl::t_v2 pos;
        zcl::t_v2 vel;

        zcl::t_i32 death_time;

        zcl::t_static_array<zcl::t_u8, 32> str_bytes;
        zcl::t_i32 str_byte_cnt;

        t_font_id font_id;
    };

    struct t_pop_up_manager {
        zcl::t_static_array<t_pop_up, k_pop_up_limit> buf;
        zcl::t_static_bitset<k_pop_up_limit> activity;
    };

    struct t_ui {
        zcl::t_i32 player_inventory_open;

        t_item_type_id cursor_held_item_type_id;
        zcl::t_i32 cursor_held_quantity;
    };

    struct t_world {
        zcl::t_rng *rng; // @note: Not sure if this should be provided externally instead?

        t_tilemap *tilemap;

        t_player_entity player_entity;
        t_player_meta player_meta;

        t_npc_manager npc_manager;

        t_item_drop_manager item_drop_manager;

        t_camera *camera;

        t_pop_up_manager pop_up_manager;

        t_ui ui;
    };

    struct t_item_type_use_func_context {
        zcl::t_v2 cursor_pos;
        zcl::t_v2_i screen_size;

        zcl::t_arena *temp_arena;

        t_tilemap *tilemap;

        t_player_meta *player_meta;
        t_player_entity *player_entity;

        t_npc_manager *npc_manager;

        t_camera *camera;

        t_pop_up_manager *pop_up_manager;

        zcl::t_rng *rng;
    };

    using t_item_type_use_func = zcl::t_b8 (*)(const t_item_type_use_func_context &context);

    extern const zcl::t_static_array<t_item_type_use_func, ekm_item_type_id_cnt> g_item_type_use_funcs;

    // ==================================================

    // ============================================================
    // @section: World Generation

    t_tilemap *GenWorld(zcl::t_rng *const rng, zcl::t_arena *const arena);

    // ==================================================

    // ============================================================
    // @section: Player

    t_player_meta CreatePlayerMeta(zcl::t_arena *const arena);

    t_player_entity CreatePlayerEntity(const t_player_meta *const player_meta, const t_tilemap *const tilemap);

    zcl::t_rect_f GetPlayerCollider(const zcl::t_v2 pos);

    void UpdatePlayerTimers(t_player_entity *const player_entity);

    void PlayerUpdateMovement(t_player_entity *const player_entity, const t_tilemap *const tilemap, const zgl::t_input_state *const input_state);

    void ProcessPlayerInventoryHotbarUpdates(t_player_meta *const player_meta, const zgl::t_input_state *const input_state);

    void ProcessPlayerItemUsage(t_world *const world, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, zcl::t_arena *const temp_arena);

    void ProcessPlayerDeath(t_player_meta *const player_meta, t_player_entity *const player_entity);

    // @note: Might want to centralise all damage handling within a single function, so we know when things like movement are applied compared to damage for example.
    void HurtPlayer(t_player_entity *const player_entity, const zcl::t_i32 damage, t_pop_up_manager *const pop_up_manager, zcl::t_rng *const rng);

    void RenderPlayer(const t_player_entity *const player_entity, const zgl::t_rendering_context rc, const t_assets *const assets);

    // ==================================================

    // ============================================================
    // @section: NPCs

    t_npc_id SpawnNPC(t_npc_manager *const manager, const zcl::t_v2 pos, const t_npc_type_id type_id);

    void HurtNPC(t_npc_manager *const manager, const t_npc_id id, const zcl::t_i32 damage);

    zcl::t_b8 CheckNPCExists(const t_npc_manager *const manager, const t_npc_id id);

    zcl::t_rect_f GetNPCCollider(const zcl::t_v2 pos, const t_npc_type_id type_id);

    void ProcessNPCAIs(t_npc_manager *const npcs, const t_tilemap *const tilemap);

    void ProcessNPCDeaths(t_npc_manager *const npcs);

    void RenderNPCs(const t_npc_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets);

    // ==================================================

    // ============================================================
    // @section: Item Drops

    void SpawnItemDrop(t_item_drop_manager *const manager, const zcl::t_v2 pos, const t_item_type_id item_type_id);

    void ProcessItemDropMovementAndCollection(t_item_drop_manager *const item_drop_manager, t_player_meta *const player_meta, const t_player_entity *const player_entity, const t_tilemap *const tilemap);

    void RenderItemDrops(const zgl::t_rendering_context rc, const t_item_drop_manager *const item_drop_manager, const t_assets *const assets);

    // ==================================================

    // ============================================================
    // @section: Pop-Ups

    t_pop_up *SpawnPopUp(t_pop_up_manager *const manager, const zcl::t_v2 pos, const zcl::t_v2 vel, const t_font_id font_id = ek_font_id_eb_garamond_32);

    t_pop_up *SpawnPopUpDamage(t_pop_up_manager *const manager, const zcl::t_v2 pos, const zcl::t_i32 damage, zcl::t_rng *const rng);

    void UpdatePopUps(t_pop_up_manager *const manager);

    // ==================================================

    // ============================================================
    // @section: UI

    void ProcessPlayerInventoryInteraction(t_ui *const ui, t_inventory *const player_inventory, const zgl::t_input_state *const input_state);

    void RenderPopUps(const zgl::t_rendering_context rc, const t_pop_up_manager *const pop_ups, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void RenderTileHighlight(const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_camera *const camera);

    void RenderPlayerInventory(const zgl::t_rendering_context rc, const t_ui *const ui, const t_player_meta *const player_meta, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void UIRenderPlayerHealth(const zgl::t_rendering_context rc, const zcl::t_i32 health, const zcl::t_i32 health_limit);

    void UIRenderPlayerDeathStr(const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void UIRenderCursorHeldItem(const t_ui *const ui, const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_assets *const assets, zcl::t_arena *const temp_arena);

    void UIRenderCursorHoverStr(const zgl::t_rendering_context rc, const zcl::t_v2 cursor_pos, const t_inventory *const player_inventory, const zcl::t_b8 player_inventory_open, const t_npc_manager *const npc_manager, const t_camera *const camera, const t_assets *const assets, zcl::t_arena *const temp_arena);

    // ==================================================
}
