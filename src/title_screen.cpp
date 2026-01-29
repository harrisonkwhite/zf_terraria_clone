#include "title_screen.h"

#include "assets.h"

constexpr zcl::t_f32 k_page_button_gap_vertical = 128.0f; // @note: This might need to be variable at some point, e.g. change based on display size.

enum t_page_id : zcl::t_i32 {
    ek_page_id_home
};

struct t_page_button {
    zcl::t_b8 disabled; // @note: Will probably need to be a function pointer at some point.
    zcl::t_v2 pos;
    zcl::t_str_rdonly str;
};

struct t_page {
    zcl::t_array_rdonly<t_page_button> buttons;
};

static t_page *PageCreate(const t_page_id id, const zcl::t_v2_i framebuffer_size, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_page>(arena);

    switch (id) {
    case ek_page_id_home: {
        const auto buttons = zcl::ArenaPushArray<t_page_button>(arena, 3);

        buttons[0] = {
            .pos = (zcl::V2IToF(framebuffer_size) / 2.0f) + zcl::t_v2{0.0f, -k_page_button_gap_vertical},
            .str = ZCL_STR_LITERAL("Start"),
        };

        buttons[1] = {
            .pos = zcl::V2IToF(framebuffer_size) / 2.0f,
            .str = ZCL_STR_LITERAL("Options"),
        };

        buttons[2] = {
            .pos = (zcl::V2IToF(framebuffer_size) / 2.0f) + zcl::t_v2{0.0f, k_page_button_gap_vertical},
            .str = ZCL_STR_LITERAL("Exit"),
        };

        result->buttons = buttons;

        break;
    }

    default:
        ZCL_UNREACHABLE();
    }

    return result;
}

struct t_title_screen {
    t_page *page_current;
    t_page_id page_current_id;
    zcl::t_arena page_current_arena;
    zcl::t_i32 page_current_button_selected_index; // -1 for no button selected.
};

t_title_screen *TitleScreenInit(const zgl::t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_title_screen>(arena);
    result->page_current = PageCreate(ek_page_id_home, zgl::WindowGetFramebufferSizeCache(platform_ticket), arena);
    result->page_current_id = ek_page_id_home;
    result->page_current_arena = zcl::ArenaCreateBlockBased();
    result->page_current_button_selected_index = -1;

    return result;
}

void TitleScreenTick(t_title_screen *const ts, const t_assets *const assets, const zgl::t_input_state *const input_state, zcl::t_arena *const temp_arena) {
    ts->page_current_button_selected_index = -1;

    for (zcl::t_i32 i = 0; i < ts->page_current->buttons.len; i++) {
        const auto btn = &ts->page_current->buttons[i];

        const auto btn_str_chr_colliders = zgl::RendererCalcStrChrColliders(btn->str, *GetFont(assets, ek_font_id_eb_garamond_48), btn->pos, temp_arena, temp_arena, zcl::k_origin_center);

        for (zcl::t_i32 j = 0; j < btn_str_chr_colliders.len; j++) {
            if (zcl::CheckPointInPoly(btn_str_chr_colliders[j], zgl::CursorGetPos(input_state))) {
                ts->page_current_button_selected_index = i;
                break;
            }
        }
    }
}

void TitleScreenRenderUI(const t_title_screen *const ts, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena) {
#if 0
    const zcl::t_v2 title_position = zcl::V2IToF(zgl::BackbufferGetSize(rc.gfx_ticket)) / 2.0f;
    zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("Terraria"), *GetFont(assets, ek_font_id_eb_garamond_128), title_position, temp_arena, zcl::k_origin_center);
#endif

    for (zcl::t_i32 i = 0; i < ts->page_current->buttons.len; i++) {
        const auto btn = &ts->page_current->buttons[i];
        const auto color = ts->page_current_button_selected_index == i ? zcl::k_color_yellow : zcl::k_color_white;

        zgl::RendererSubmitStr(rc, btn->str, *GetFont(assets, ek_font_id_eb_garamond_48), btn->pos, color, temp_arena, zcl::k_origin_center);
    }
}

void TitleScreenProcessBackbufferResize(t_title_screen *const ts, const zcl::t_v2_i backbuffer_size) {
    zcl::ArenaRewind(&ts->page_current_arena);
    ts->page_current = PageCreate(ek_page_id_home, backbuffer_size, &ts->page_current_arena);
    ts->page_current_button_selected_index = -1;
}
