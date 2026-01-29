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
