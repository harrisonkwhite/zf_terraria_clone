#include "ui.h"

struct t_page {
    zcl::t_array_rdonly<t_page_elem> elems;
};

t_page *PageCreate(const zcl::t_v2_i size, const zcl::t_array_rdonly<t_page_elem> elems, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_page>(arena);
    result->elems = elems;

    return result;
}

void PageUpdate(t_page *const page, const zcl::t_v2 cursor_position, const zcl::t_b8 mouse_button_pressed, zcl::t_arena *const temp_arena) {
    for (zcl::t_i32 i = 0; i < page->elems.len; i++) {
        const auto elem = &page->elems[i];

        switch (elem->type_id) {
        case ek_page_elem_type_id_button: {
            const auto btn_str_chr_colliders = zgl::RendererCalcStrChrColliders(elem->type_data.button.str, *elem->type_data.button.font, elem->position, temp_arena, temp_arena, zcl::k_origin_center);

            for (zcl::t_i32 j = 0; j < btn_str_chr_colliders.len; j++) {
                if (zcl::CheckPointInPoly(btn_str_chr_colliders[j], cursor_position)) {
                    if (elem->type_data.button.click_func && mouse_button_pressed) {
                        elem->type_data.button.click_func(elem->type_data.button.click_func_data);
                    }

                    break;
                }
            }

            break;
        }

        case ek_page_elem_type_id_slot:
            break;
        }
    }

#if 0
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

    if (btn_hovered_index != -1 && mouse_button_pressed && page->buttons[btn_hovered_index].click_func) {
        page->buttons[btn_hovered_index].click_func(page->buttons[btn_hovered_index].click_func_data);
    }
#endif
}

void PageRender(const t_page *const page, const zgl::t_rendering_context rendering_context, zcl::t_arena *const temp_arena) {
    for (zcl::t_i32 i = 0; i < page->elems.len; i++) {
        const auto elem = &page->elems[i];

        switch (elem->type_id) {
        case ek_page_elem_type_id_button:
            zgl::RendererSubmitStr(rendering_context, elem->type_data.button.str, *elem->type_data.button.font, elem->position, zcl::k_color_white, temp_arena, zcl::k_origin_center);
            break;

        case ek_page_elem_type_id_slot:
            zgl::RendererSubmitRectOutlineOpaque(rendering_context, zcl::RectCreateF(elem->position - (elem->type_data.slot.size / 2.0f), elem->type_data.slot.size), 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            break;
        }
    }
}
