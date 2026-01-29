#include "ui.h"

constexpr zcl::t_f32 k_page_button_hover_scale_offs_targ = 0.15f;
constexpr zcl::t_f32 k_page_button_hover_scale_offs_lerp_factor = 0.2f;

struct t_page_button_dynamic {
    zcl::t_f32 scale_offs;
};

struct t_page {
    zcl::t_array_rdonly<t_page_button> buttons;
    zcl::t_array_mut<t_page_button_dynamic> buttons_dynamic;
};

t_page *PageCreate(const zcl::t_v2_i size, const zcl::t_array_rdonly<t_page_button> buttons, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_page>(arena);
    result->buttons = buttons;
    result->buttons_dynamic = zcl::ArenaPushArray<t_page_button_dynamic>(arena, buttons.len);
    return result;
}

void PageUpdate(t_page *const page, const zcl::t_v2 cursor_position, const zcl::t_b8 mouse_button_pressed, zcl::t_arena *const temp_arena) {
    zcl::t_i32 btn_hovered_index = -1;

    for (zcl::t_i32 i = 0; i < page->buttons.len; i++) {
        const auto btn = &page->buttons[i];
        const auto btn_dynamic = &page->buttons_dynamic[i];

        const auto btn_str_chr_colliders = zgl::RendererCalcStrChrColliders(btn->str, *btn->font, btn->position, temp_arena, temp_arena, zcl::k_origin_center);

        zcl::t_f32 btn_scale_offs_targ = 0.0f;

        for (zcl::t_i32 j = 0; j < btn_str_chr_colliders.len; j++) {
            if (zcl::CheckPointInPoly(btn_str_chr_colliders[j], cursor_position)) {
                btn_hovered_index = i;
                btn_scale_offs_targ = k_page_button_hover_scale_offs_targ;
                break;
            }
        }

        btn_dynamic->scale_offs = zcl::Lerp(btn_dynamic->scale_offs, btn_scale_offs_targ, k_page_button_hover_scale_offs_lerp_factor);
    }

    if (btn_hovered_index != -1 && mouse_button_pressed && page->buttons[btn_hovered_index].callback_func) {
        page->buttons[btn_hovered_index].callback_func(page->buttons[btn_hovered_index].callback_func_data);
    }
}

void PageRender(const t_page *const page, const zgl::t_rendering_context rendering_context, zcl::t_arena *const temp_arena) {
    for (zcl::t_i32 i = 0; i < page->buttons.len; i++) {
        const auto btn = &page->buttons[i];
        const auto btn_dynamic = &page->buttons_dynamic[i];

        const zcl::t_v2 btn_scale = {1.0f + btn_dynamic->scale_offs, 1.0f + btn_dynamic->scale_offs};

        zgl::RendererSubmitStr(rendering_context, btn->str, *btn->font, btn->position, zcl::k_color_white, temp_arena, zcl::k_origin_center, 0.0f, btn_scale);
    }
}
