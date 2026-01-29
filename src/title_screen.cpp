#include "title_screen.h"

#include "assets.h"
#include "ui.h"

enum t_title_screen_page_id : zcl::t_i32 {
    ek_title_screen_page_id_home,
    ek_title_screen_page_id_options
};

struct t_title_screen {
    t_page *page_current;
    t_title_screen_page_id page_current_id;
    zcl::t_arena page_current_arena;
};

constexpr zcl::t_f32 k_title_screen_page_button_gap_vertical = 96.0f;

static t_page *TitleScreenPageCreate(const t_title_screen_page_id id, const zcl::t_v2_i size, const t_assets *const assets, zcl::t_arena *const arena) {
    switch (id) {
    case ek_title_screen_page_id_home: {
        const auto buttons = zcl::ArenaPushArray<t_page_button>(arena, 3);

        buttons[0] = {
            .position = (zcl::V2IToF(size) / 2.0f) + zcl::t_v2{0.0f, -k_title_screen_page_button_gap_vertical},
            .str = ZCL_STR_LITERAL("Start"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
        };

        buttons[1] = {
            .position = zcl::V2IToF(size) / 2.0f,
            .str = ZCL_STR_LITERAL("Options"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
        };

        buttons[2] = {
            .position = (zcl::V2IToF(size) / 2.0f) + zcl::t_v2{0.0f, k_title_screen_page_button_gap_vertical},
            .str = ZCL_STR_LITERAL("Exit"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
        };

        return UIPageCreate(size, buttons, arena);
    }

    case ek_title_screen_page_id_options: {
        const auto buttons = zcl::ArenaPushArray<t_page_button>(arena, 1);

        buttons[0] = {
            .position = zcl::V2IToF(size) / 2.0f,
            .str = ZCL_STR_LITERAL("Back"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
        };

        return UIPageCreate(size, buttons, arena);
    }

    default:
        ZCL_UNREACHABLE();
    }
}

t_title_screen *TitleScreenInit(const t_assets *const assets, const zgl::t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_title_screen>(arena);
    result->page_current = TitleScreenPageCreate(ek_title_screen_page_id_home, zgl::WindowGetFramebufferSizeCache(platform_ticket), assets, arena);
    result->page_current_id = ek_title_screen_page_id_home;
    result->page_current_arena = zcl::ArenaCreateBlockBased();

    return result;
}

t_title_screen_tick_result_id TitleScreenTick(t_title_screen *const ts, const t_assets *const assets, const zgl::t_input_state *const input_state, const zgl::t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const temp_arena) {
    t_title_screen_tick_result_id result = ek_title_screen_tick_result_id_normal;

    UIPageUpdate(ts->page_current, zgl::CursorGetPos(input_state), temp_arena);

    return result;
}

void TitleScreenRenderUI(const t_title_screen *const ts, const zgl::t_rendering_context rendering_context, const t_assets *const assets, zcl::t_arena *const temp_arena) {
#if 0
    const zcl::t_v2 title_position = zcl::V2IToF(zgl::BackbufferGetSize(rc.gfx_ticket)) / 2.0f;
    zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("Terraria"), *GetFont(assets, ek_font_id_eb_garamond_128), title_position, temp_arena, zcl::k_origin_center);
#endif

    UIPageRender(ts->page_current, rendering_context, temp_arena);
}

void TitleScreenProcessBackbufferResize(t_title_screen *const ts, const zcl::t_v2_i backbuffer_size, const t_assets *const assets) {
    zcl::ArenaRewind(&ts->page_current_arena);
    ts->page_current = TitleScreenPageCreate(ek_title_screen_page_id_home, backbuffer_size, assets, &ts->page_current_arena);
}
