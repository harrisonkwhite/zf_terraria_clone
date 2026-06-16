#include "ui_helpers.h"

constexpr zcl::t_f32 k_button_hover_scale_offs = 0.1f;
constexpr zcl::t_f32 k_button_hover_scale_offs_lerp_factor = 0.2f;

struct t_page_elem_dynamic {
    // @todo: So these are both awkwardly specific to buttons...
    zcl::t_b8 hovered;
    zcl::t_f32 scale_offs;
};

struct t_page {
    zcl::t_array_rdonly<t_page_elem_static> elem_statics;
    zcl::t_array_mut<t_page_elem_dynamic> elem_dynamics;
};

t_page *PageCreate(const zcl::t_v2_i size, const zcl::t_array_rdonly<t_page_elem_static> elem_statics, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_page>(arena);
    result->elem_statics = elem_statics;
    result->elem_dynamics = zcl::ArenaPushArray<t_page_elem_dynamic>(arena, elem_statics.len);

    return result;
}

void PageUpdate(t_page *const page, const zcl::t_v2 cursor_position, const zcl::t_b8 mouse_button_pressed, zcl::t_arena *const temp_arena) {
    for (zcl::t_i32 i = 0; i < page->elem_statics.len; i++) {
        const auto elem_static = &page->elem_statics[i];

        const auto elem_dynamic = &page->elem_dynamics[i];
        elem_dynamic->hovered = false;

        switch (elem_static->type_id) {
            case ek_page_elem_type_id_button: {
                const auto btn_str_collider = zgl::CalcStrRenderCollider(elem_static->type_data.button.str, *elem_static->type_data.button.font, elem_static->position, temp_arena, temp_arena, zcl::k_origin_center);

                if (zcl::CheckPointInPoly(btn_str_collider, cursor_position)) {
                    elem_dynamic->hovered = true;

                    if (elem_static->type_data.button.click_func && mouse_button_pressed) {
                        elem_static->type_data.button.click_func(elem_static->type_data.button.click_func_data);
                    }
                }

                break;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }

        const zcl::t_f32 scale_offs_targ = elem_dynamic->hovered ? k_button_hover_scale_offs : 0.0f;
        elem_dynamic->scale_offs = zcl::Lerp(elem_dynamic->scale_offs, scale_offs_targ, k_button_hover_scale_offs_lerp_factor);
    }
}

void PageRender(const t_page *const page, const zgl::t_rendering_context rc, zcl::t_arena *const temp_arena) {
    for (zcl::t_i32 i = 0; i < page->elem_statics.len; i++) {
        const auto elem_static = &page->elem_statics[i];
        const auto elem_dynamic = &page->elem_dynamics[i];

        switch (elem_static->type_id) {
            case ek_page_elem_type_id_button: {
                zgl::RendererSubmitStr(rc, elem_static->type_data.button.str, *elem_static->type_data.button.font, elem_static->position, elem_dynamic->hovered ? zcl::k_color_yellow : zcl::k_color_white, temp_arena, zcl::k_origin_center, 0.0f, {1.0f + elem_dynamic->scale_offs, 1.0f + elem_dynamic->scale_offs});

                break;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }
    }
}
