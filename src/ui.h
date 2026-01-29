#pragma once

struct t_page;

struct t_page_button {
    zcl::t_v2 position;
    zcl::t_str_rdonly str;
    const zgl::t_font *font;
};

t_page *UIPageCreate(const zcl::t_v2_i size, const zcl::t_array_rdonly<t_page_button> buttons, zcl::t_arena *const arena);
void UIPageUpdate(t_page *const page, const zcl::t_v2 cursor_position, zcl::t_arena *const temp_arena);
void UIPageRender(const t_page *const page, const zgl::t_rendering_context rendering_context, zcl::t_arena *const temp_arena);
