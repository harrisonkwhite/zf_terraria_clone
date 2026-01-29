#include "title_screen.h"

#include "assets.h"

// @todo: So the current system is problematic because you might have the "button selected" state for example persist even after a backbuffer resize occurs between frames (or maybe this isn't a problem).
//        Will probably want to .

constexpr zcl::t_f32 k_page_button_gap_vertical = 64.0f; // @note: This might need to be variable at some point, e.g. change based on display size.

enum t_page_id : zcl::t_i32 {
    ek_page_id_home
};

struct t_page_button {
    zcl::t_b8 disabled; // @note: Will probably need to be a function pointer at some point.
    zcl::t_v2 (*pos_calculator)(const zcl::t_v2_i framebuffer_size);
    zcl::t_str_rdonly str;
};

struct t_title_screen {
    t_page_id page_current_id;
    zcl::t_arena page_current_arena;
    zcl::t_array_mut<t_page_button> page_current_buttons;
    zcl::t_i32 page_current_button_selected_index; // -1 for no button selected.
};

static void PageSwitch(t_title_screen *const title_screen, const t_page_id page_id) {
    title_screen->page_current_id = ek_page_id_home;

    zcl::ArenaRewind(&title_screen->page_current_arena);

    switch (page_id) {
    case ek_page_id_home:
        title_screen->page_current_buttons = zcl::ArenaPushArray<t_page_button>(&title_screen->page_current_arena, 3);

        title_screen->page_current_buttons[0] = {
            .pos_calculator = [](const zcl::t_v2_i framebuffer_size) -> zcl::t_v2 {
                return (zcl::V2IToF(framebuffer_size) / 2.0f) + zcl::t_v2{0.0f, -k_page_button_gap_vertical};
            },
            .str = ZCL_STR_LITERAL("Start"),
        };

        title_screen->page_current_buttons[1] = {
            .pos_calculator = [](const zcl::t_v2_i framebuffer_size) -> zcl::t_v2 {
                return (zcl::V2IToF(framebuffer_size) / 2.0f);
            },
            .str = ZCL_STR_LITERAL("Start"),
        };

        title_screen->page_current_buttons[2] = {
            .pos_calculator = [](const zcl::t_v2_i framebuffer_size) -> zcl::t_v2 {
                return (zcl::V2IToF(framebuffer_size) / 2.0f) + zcl::t_v2{0.0f, k_page_button_gap_vertical};
            },
            .str = ZCL_STR_LITERAL("Start"),
        };

        break;

    default:
        ZCL_UNREACHABLE();
    }

    title_screen->page_current_button_selected_index = -1;
}

t_title_screen *TitleScreenInit(zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_title_screen>(arena);
    result->page_current_arena = zcl::ArenaCreateBlockBased();
    result->page_current_button_selected_index = -1;

    PageSwitch(result, ek_page_id_home);

    return result;
}

void TitleScreenTick(t_title_screen *const ts) {
}

void TitleScreenRenderUI(const t_title_screen *const ts, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 title_position = zcl::V2IToF(zgl::BackbufferGetSize(rc.gfx_ticket)) / 2.0f;
    zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("Terraria"), *GetFont(assets, ek_font_id_eb_garamond_128), title_position, temp_arena, zcl::k_origin_center);

    for (zcl::t_i32 i = 0; i < ts->page_current_buttons.len; i++) {
        const auto btn = &ts->page_current_buttons[i];
        zgl::RendererSubmitStr(rc, btn->str, *GetFont(assets, ek_font_id_eb_garamond_48), btn->pos_calculator(zgl::BackbufferGetSize(rc.gfx_ticket)), temp_arena, zcl::k_origin_center);
    }
}
