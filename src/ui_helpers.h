#pragma once

#if 0
// @todo: This whole page thing is probably an overabstraction?

struct t_page;

enum t_page_elem_type_id : zcl::t_i32 {
    ek_page_elem_type_id_button
};

struct t_page_elem_static {
    zcl::t_v2 position;

    t_page_elem_type_id type_id;

    union {
        struct {
            zcl::t_str_rdonly str;

            const zgl::t_font *font;

            void (*click_func)(void *const func_data);
            void *click_func_data;
        } button;
    } type_data;
};

t_page *PageCreate(const zcl::t_v2_i size, const zcl::t_array_rdonly<t_page_elem_static> elem_statics, zcl::t_arena *const arena);
void PageUpdate(t_page *const page, const zcl::t_v2 cursor_position, const zcl::t_b8 mouse_button_pressed, zcl::t_arena *const temp_arena);
void PageRender(const t_page *const page, const zgl::t_rendering_context rc, zcl::t_arena *const temp_arena);
#endif

inline void RenderStrWithOutline(const zgl::t_rendering_context rc, const zcl::t_str_rdonly str, const zgl::t_font &font, const zcl::t_v2 pos, const zcl::t_color_rgba32f color, zcl::t_arena *const temp_arena, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f, const zcl::t_v2 scale = {1.0f, 1.0f}) {
    constexpr zcl::t_f32 k_outline_thickness = 2.0f;
    const zcl::t_color_rgba32f outline_color = {0.0f, 0.0f, 0.0f, color.a};

    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{k_outline_thickness, 0.0f}, outline_color, temp_arena, origin, rot, scale);
    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{-k_outline_thickness, 0.0f}, outline_color, temp_arena, origin, rot, scale);
    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{0.0f, k_outline_thickness}, outline_color, temp_arena, origin, rot, scale);
    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{0.0f, -k_outline_thickness}, outline_color, temp_arena, origin, rot, scale);

    zgl::RendererSubmitStr(rc, str, font, pos, color, temp_arena, origin, rot, scale);
}
