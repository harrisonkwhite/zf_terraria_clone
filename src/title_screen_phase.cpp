#include "title_screen_phase.h"

#include "assets.h"
#include "ui_helpers.h"
#include "options.h"

constexpr zcl::t_f32 k_title_screen_logo_wave_rot_acc = 0.01f;
constexpr zcl::t_f32 k_title_screen_logo_wave_rot_mult = 0.01f * zcl::k_pi;
constexpr zcl::t_f32 k_title_screen_logo_wave_scale_offs_acc = 0.015f;
constexpr zcl::t_f32 k_title_screen_logo_wave_scale_offs_mult = 0.04f;

constexpr t_font_id k_title_screen_button_font_id = ek_font_id_roboto_40;
constexpr zcl::t_f32 k_title_screen_button_hover_scale_offs = 0.1f;
constexpr zcl::t_f32 k_title_screen_button_hover_scale_offs_lerp_factor = 0.2f;

constexpr t_font_id k_title_screen_option_font_id = ek_font_id_roboto_28;

enum t_title_screen_page_id : zcl::t_i32 {
    ek_title_screen_page_id_home,
    ek_title_screen_page_id_options
};

enum t_title_screen_page_request_type_id : zcl::t_i32 {
    ek_title_screen_page_request_type_id_switch_page,
    ek_title_screen_page_request_type_id_go_to_world,
    ek_title_screen_page_request_type_id_exit_game
};

struct t_title_screen_page_request {
    t_title_screen_page_request_type_id type_id;

    union {
        struct {
            t_title_screen_page_id page_id;
        } switch_page;
    } type_data;
};

enum t_title_screen_page_elem_type_id : zcl::t_i32 {
    ek_title_screen_page_elem_type_id_button,
    ek_title_screen_page_elem_type_id_option,

    ekm_title_screen_page_elem_type_id_cnt
};

constexpr zcl::t_f32 k_title_screen_page_elems_center_y_screen_mult = 0.5825f; // What percentage of the screen height is the center Y?

constexpr zcl::t_static_array<zcl::t_f32, ekm_title_screen_page_elem_type_id_cnt> k_title_screen_page_elem_type_paddings_y = {{
    48.0f,
    24.0f,
}};

struct t_title_screen_page_elem_static {
    t_title_screen_page_elem_type_id type_id;

    union {
        struct {
            zcl::t_str_rdonly str;
            void (*click_func)(zcl::t_list<t_title_screen_page_request> *const requests);
        } button;

        struct {
            t_option_id id;
        } option;
    } type_data;
};

struct t_title_screen_page_elem_dynamic {
    union {
        struct {
            zcl::t_b8 hovered;
            zcl::t_f32 scale_offs;
        } button;
    } type_data;
};

struct t_title_screen_page {
    zcl::t_array_rdonly<t_title_screen_page_elem_static> elem_statics;
    zcl::t_array_mut<t_title_screen_page_elem_dynamic> elem_dynamics;
};

struct t_title_screen_phase {
    zcl::t_f32 logo_wave_rot;
    zcl::t_f32 logo_wave_scale_offs;

    t_title_screen_page_id page_id;
    zcl::t_arena *page_arena;
    t_title_screen_page page;
};

static t_title_screen_page TitleScreenPageCreate(const t_title_screen_page_id id, const zcl::t_v2_i size, zcl::t_arena *const arena) {
    zcl::t_array_mut<t_title_screen_page_elem_static> elem_statics;

    switch (id) {
        case ek_title_screen_page_id_home: {
            elem_statics = zcl::ArenaPushArray<t_title_screen_page_elem_static>(arena, 3);

            elem_statics[0] = {
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Start"),
                        .click_func = [](zcl::t_list<t_title_screen_page_request> *const requests) {
                            const auto request = t_title_screen_page_request{
                                .type_id = ek_title_screen_page_request_type_id_go_to_world,
                            };

                            zcl::ListAppend(requests, request);
                        },
                    },
                },
            };

            elem_statics[1] = {
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Options"),
                        .click_func = [](zcl::t_list<t_title_screen_page_request> *const requests) {
                            const auto request = t_title_screen_page_request{
                                .type_id = ek_title_screen_page_request_type_id_switch_page,
                                .type_data = {
                                    .switch_page = {
                                        .page_id = ek_title_screen_page_id_options,
                                    },
                                },
                            };

                            zcl::ListAppend(requests, request);
                        },
                    },
                },
            };

            elem_statics[2] = {
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Exit"),
                        .click_func = [](zcl::t_list<t_title_screen_page_request> *const requests) {
                            const auto request = t_title_screen_page_request{
                                .type_id = ek_title_screen_page_request_type_id_exit_game,
                            };

                            zcl::ListAppend(requests, request);
                        },
                    },
                },
            };

            break;
        }

        case ek_title_screen_page_id_options: {
            elem_statics = zcl::ArenaPushArray<t_title_screen_page_elem_static>(arena, g_option_metas.k_len + 1);

            for (zcl::t_i32 i = 0; i < g_option_metas.k_len; i++) {
                elem_statics[i] = {
                    .type_id = ek_title_screen_page_elem_type_id_option,
                    .type_data = {
                        .option = {
                            .id = static_cast<t_option_id>(i),
                        },
                    },
                };
            }

            elem_statics[g_option_metas.k_len] = {
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Back"),
                        .click_func = [](zcl::t_list<t_title_screen_page_request> *const requests) {
                            const auto request = t_title_screen_page_request{
                                .type_id = ek_title_screen_page_request_type_id_switch_page,
                                .type_data = {
                                    .switch_page = {
                                        .page_id = ek_title_screen_page_id_home,
                                    },
                                },
                            };

                            zcl::ListAppend(requests, request);
                        },
                    },
                },
            };

            break;
        }

        default: {
            ZCL_UNREACHABLE();
        }
    }

    return {
        .elem_statics = elem_statics,
        .elem_dynamics = zcl::ArenaPushArray<t_title_screen_page_elem_dynamic>(arena, elem_statics.len),
    };
}

static zcl::t_array_mut<zcl::t_v2> TitleScreenPageElemsLoadPositions(const zcl::t_array_rdonly<t_title_screen_page_elem_static> elem_statics, const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
    const zcl::t_f32 x = screen_size.x / 2.0f;

    const auto result = zcl::ArenaPushArray<zcl::t_v2>(arena, elem_statics.len);

    zcl::t_f32 y_pen = 0.0f;

    for (zcl::t_i32 i = 0; i < elem_statics.len; i++) {
        y_pen += k_title_screen_page_elem_type_paddings_y[elem_statics[i].type_id];
        result[i] = {x, y_pen};
        y_pen += k_title_screen_page_elem_type_paddings_y[elem_statics[i].type_id];
    }

    const zcl::t_f32 height = y_pen;

    for (zcl::t_i32 i = 0; i < elem_statics.len; i++) {
        result[i].y += (screen_size.y * k_title_screen_page_elems_center_y_screen_mult) - (height / 2.0f);
    }

    return result;
}

t_title_screen_phase *TitleScreenPhaseInit(const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_title_screen_phase>(arena);

    result->page_id = ek_title_screen_page_id_home;

    const auto page_arena_mem = zcl::ArenaPushArray<zcl::t_u8>(arena, zcl::KilobytesToBytes(1));
    result->page_arena = zcl::ArenaCreateWrapping(page_arena_mem);

    result->page = TitleScreenPageCreate(result->page_id, screen_size, result->page_arena);

    return result;
}

t_title_screen_phase_tick_result_id TitleScreenPhaseTick(t_title_screen_phase *const ts, t_options *const options, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena) {
    t_title_screen_phase_tick_result_id result = ek_title_screen_phase_tick_result_id_normal;

    const auto cursor_position = zgl::CursorGetPos(input_state);
    const auto mouse_button_down = zgl::MouseButtonCheckDown(input_state, zgl::ek_mouse_button_code_left);
    const auto mouse_button_pressed = zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left);

    // Update logo.
    ts->logo_wave_rot += k_title_screen_logo_wave_rot_acc;

    while (ts->logo_wave_rot > 2.0f * zcl::k_pi) {
        ts->logo_wave_rot -= 2.0f * zcl::k_pi;
    }

    ts->logo_wave_scale_offs += k_title_screen_logo_wave_scale_offs_acc;

    while (ts->logo_wave_scale_offs > 2.0f * zcl::k_pi) {
        ts->logo_wave_scale_offs -= 2.0f * zcl::k_pi;
    }

    // Update page elements.
    const auto page_elem_positions = TitleScreenPageElemsLoadPositions(ts->page.elem_statics, screen_size, temp_arena);

    auto page_requests = zcl::ListCreate<t_title_screen_page_request>(32, temp_arena);

    for (zcl::t_i32 i = 0; i < ts->page.elem_statics.len; i++) {
        const auto elem_static = &ts->page.elem_statics[i];

        const auto elem_dynamic = &ts->page.elem_dynamics[i];
        elem_dynamic->type_data.button.hovered = false;

        switch (elem_static->type_id) {
            case ek_title_screen_page_elem_type_id_button: {
                const auto btn_str_collider = zgl::CalcStrRenderCollider(elem_static->type_data.button.str, *FontGet(assets, k_title_screen_button_font_id), page_elem_positions[i], temp_arena, temp_arena, zcl::k_origin_center);

                if (zcl::CheckPointInPoly(btn_str_collider, cursor_position)) {
                    elem_dynamic->type_data.button.hovered = true;

                    if (mouse_button_pressed) {
                        ZCL_ASSERT(elem_static->type_data.button.click_func);
                        elem_static->type_data.button.click_func(&page_requests);

                        zgl::SoundFireAndForget(audio_ticket, SoundTypeGet(assets, ek_sound_type_id_button_click));
                    }
                }

                break;
            }

            case ek_title_screen_page_elem_type_id_option: {
                break;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }

        const zcl::t_f32 scale_offs_targ = elem_dynamic->type_data.button.hovered ? k_title_screen_button_hover_scale_offs : 0.0f;
        elem_dynamic->type_data.button.scale_offs = zcl::Lerp(elem_dynamic->type_data.button.scale_offs, scale_offs_targ, k_title_screen_button_hover_scale_offs_lerp_factor);
    }

    // Process page requests.
    for (zcl::t_i32 i = 0; i < page_requests.len; i++) {
        const auto request = &page_requests[i];

        switch (request->type_id) {
            case ek_title_screen_page_request_type_id_switch_page: {
                zcl::ArenaRewind(ts->page_arena);
                ts->page_id = request->type_data.switch_page.page_id;
                ts->page = TitleScreenPageCreate(ts->page_id, screen_size, ts->page_arena);
                break;
            }

            case ek_title_screen_page_request_type_id_go_to_world: {
                result = ek_title_screen_phase_tick_result_id_go_to_world;
                break;
            }

            case ek_title_screen_page_request_type_id_exit_game: {
                result = ek_title_screen_phase_tick_result_id_exit_game;
                break;
            }
        }
    }

    return result;
}

void TitleScreenPhaseRenderUI(const t_title_screen_phase *const ts, const zgl::t_rendering_context rc, const t_options *const options, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    // Render page elements.
    const auto page_elem_positions = TitleScreenPageElemsLoadPositions(ts->page.elem_statics, rc.screen_size, temp_arena);

    for (zcl::t_i32 i = 0; i < ts->page.elem_statics.len; i++) {
        const auto elem_static = &ts->page.elem_statics[i];
        const auto elem_dynamic = &ts->page.elem_dynamics[i];

        switch (elem_static->type_id) {
            case ek_title_screen_page_elem_type_id_button: {
                RenderStrWithOutline(rc, elem_static->type_data.button.str, *FontGet(assets, k_title_screen_button_font_id), page_elem_positions[i], elem_dynamic->type_data.button.hovered ? zcl::k_color_yellow : zcl::k_color_white, temp_arena, zcl::k_origin_center, 0.0f, {1.0f + elem_dynamic->type_data.button.scale_offs, 1.0f + elem_dynamic->type_data.button.scale_offs});
                break;
            }

            case ek_title_screen_page_elem_type_id_option: {
                // Render option name (left).
                {
                    zcl::t_static_array<zcl::t_u8, 32> str_bytes = {};

                    auto str_bytes_stream = zcl::ByteStreamCreate(str_bytes, zcl::ek_stream_mode_write);
                    zcl::PrintFormat(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("%:"), g_option_metas[elem_static->type_data.option.id].name);

                    const auto str = zcl::t_str_rdonly{zcl::ByteStreamGetWritten(&str_bytes_stream)};
                    const auto str_pos = page_elem_positions[i] + zcl::t_v2{-256.0f, 0.0f};

                    RenderStrWithOutline(rc, str, *FontGet(assets, k_title_screen_option_font_id), str_pos, zcl::k_color_white, temp_arena, zcl::k_origin_center_left);
                }

                // Render option value (right).
                {
                    const auto str_pos = page_elem_positions[i] + zcl::t_v2{256.0f, 0.0f};

                    zcl::t_static_array<zcl::t_u8, 32> str_bytes = {};

                    auto str_bytes_stream = zcl::ByteStreamCreate(str_bytes, zcl::ek_stream_mode_write);

                    switch (g_option_metas[elem_static->type_data.option.id].type_id) {
                        case ek_option_type_id_perc: {
                            zcl::PrintFormat(zcl::ByteStreamGetView(&str_bytes_stream), ZCL_STR_LITERAL("%^%"), static_cast<zcl::t_i32>(zcl::Floor(OptionsGetPerc(options, elem_static->type_data.option.id) * 100.0f)));
                            break;
                        }

                        case ek_option_type_id_toggle: {
                            zcl::PrintFormat(zcl::ByteStreamGetView(&str_bytes_stream), OptionsGetToggle(options, elem_static->type_data.option.id) ? ZCL_STR_LITERAL("Enabled") : ZCL_STR_LITERAL("Disabled"));
                            break;
                        }
                    }

                    const auto str = zcl::t_str_rdonly{zcl::ByteStreamGetWritten(&str_bytes_stream)};
                    const auto str_origin = zcl::k_origin_center_right;
                    const auto str_collider = zgl::CalcStrRenderCollider(str, *FontGet(assets, k_title_screen_option_font_id), str_pos, temp_arena, temp_arena, str_origin);
                    const auto str_collider_spanning_rect = zcl::CalcSpanningRect(str_collider);

                    RenderStrWithOutline(rc, str, *FontGet(assets, k_title_screen_option_font_id), str_pos, zcl::k_color_white, temp_arena, str_origin);

                    // Render left and right buttons.
                    {
                        constexpr zcl::t_v2 k_arrow_size = {8.0f, 8.0f};
                        constexpr zcl::t_f32 k_arrow_x_offs = 16.0f;
                        const zcl::t_f32 arrow_y = str_collider_spanning_rect.y + (str_collider_spanning_rect.height / 2.0f);

                        {
                            const zcl::t_v2 left_arrow_pos = {str_collider_spanning_rect.x - k_arrow_x_offs, arrow_y};

                            const zcl::t_static_array<zcl::t_v2, 3> left_arrow_pts = {{
                                {left_arrow_pos.x - (k_arrow_size.x / 2.0f), left_arrow_pos.y},
                                {left_arrow_pos.x + (k_arrow_size.x / 2.0f), left_arrow_pos.y - (k_arrow_size.y / 2.0f)},
                                {left_arrow_pos.x + (k_arrow_size.x / 2.0f), left_arrow_pos.y + (k_arrow_size.y / 2.0f)},
                            }};

                            zgl::RendererSubmitTriangle(rc, left_arrow_pts, zcl::k_color_white);
                        }

                        {
                            const zcl::t_v2 right_arrow_pos = {str_collider_spanning_rect.x + str_collider_spanning_rect.width + k_arrow_x_offs, arrow_y};

                            const zcl::t_static_array<zcl::t_v2, 3> right_arrow_pts = {{
                                {right_arrow_pos.x + (k_arrow_size.x / 2.0f), right_arrow_pos.y},
                                {right_arrow_pos.x - (k_arrow_size.x / 2.0f), right_arrow_pos.y - (k_arrow_size.y / 2.0f)},
                                {right_arrow_pos.x - (k_arrow_size.x / 2.0f), right_arrow_pos.y + (k_arrow_size.y / 2.0f)},
                            }};

                            zgl::RendererSubmitTriangle(rc, right_arrow_pts, zcl::k_color_white);
                        }
                    }
                }

                break;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }
    }

    // Render the logo.
    const zcl::t_v2 logo_position = {rc.screen_size.x * 0.5f, rc.screen_size.y * 0.2f};
    const zcl::t_f32 logo_rot = zcl::Sin(ts->logo_wave_rot) * k_title_screen_logo_wave_rot_mult;
    const zcl::t_f32 logo_scale_offs = zcl::Sin(ts->logo_wave_scale_offs) * k_title_screen_logo_wave_scale_offs_mult;
    RenderStrWithOutline(rc, ZCL_STR_LITERAL("Terraria"), *FontGet(assets, ek_font_id_roboto_184), logo_position, zcl::k_color_white, temp_arena, zcl::k_origin_center, logo_rot, {1.0f - k_title_screen_logo_wave_scale_offs_mult + logo_scale_offs, 1.0f - k_title_screen_logo_wave_scale_offs_mult + logo_scale_offs});
}

void TitleScreenPhaseProcessScreenResize(t_title_screen_phase *const ts, const zcl::t_v2_i screen_size) {
    // Recreate the page with the new size.
    zcl::ArenaRewind(ts->page_arena);
    ts->page = TitleScreenPageCreate(ts->page_id, screen_size, ts->page_arena);
}
