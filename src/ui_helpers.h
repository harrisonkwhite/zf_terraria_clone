#pragma once

inline void RenderStrWithOutline(const zgl::t_rendering_context rc, const zcl::t_str_rdonly str, const zgl::t_font &font, const zcl::t_v2 pos, const zcl::t_color_rgba32f color, zcl::t_arena *const temp_arena, const zcl::t_v2 origin = zcl::k_origin_top_left, const zcl::t_f32 rot = 0.0f, const zcl::t_v2 scale = {1.0f, 1.0f}) {
    constexpr zcl::t_f32 k_outline_thickness = 2.0f;
    const zcl::t_color_rgba32f outline_color = {0.0f, 0.0f, 0.0f, color.a};

    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{k_outline_thickness, 0.0f}, outline_color, temp_arena, origin, rot, scale);
    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{-k_outline_thickness, 0.0f}, outline_color, temp_arena, origin, rot, scale);
    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{0.0f, k_outline_thickness}, outline_color, temp_arena, origin, rot, scale);
    zgl::RendererSubmitStr(rc, str, font, pos + zcl::t_v2{0.0f, -k_outline_thickness}, outline_color, temp_arena, origin, rot, scale);

    zgl::RendererSubmitStr(rc, str, font, pos, color, temp_arena, origin, rot, scale);
}
