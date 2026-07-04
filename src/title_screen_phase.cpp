#include "title_screen_phase.h"

#include "assets.h"
#include "ui_helpers.h"

constexpr zcl::t_f32 k_title_screen_logo_wave_rot_acc = 0.01f;
constexpr zcl::t_f32 k_title_screen_logo_wave_rot_mult = 0.01f * zcl::k_pi;
constexpr zcl::t_f32 k_title_screen_logo_wave_scale_offs_acc = 0.015f;
constexpr zcl::t_f32 k_title_screen_logo_wave_scale_offs_mult = 0.04f;

constexpr zcl::t_f32 k_title_screen_button_hover_scale_offs = 0.1f;
constexpr zcl::t_f32 k_title_screen_button_hover_scale_offs_lerp_factor = 0.2f;

enum t_title_screen_page_id : zcl::t_i32 {
    ek_title_screen_page_id_home,
    ek_title_screen_page_id_options
};

enum t_title_screen_page_request_type_id : zcl::t_i32 {
    ek_title_screen_page_request_type_id_switch_page,
    ek_title_screen_page_request_type_id_go_to_world,
    ek_title_screen_page_request_type_id_exit_game,
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
    ek_title_screen_page_elem_type_id_button
};

struct t_title_screen_page_elem_static {
    zcl::t_v2 position;

    t_title_screen_page_elem_type_id type_id;

    union {
        struct {
            zcl::t_str_rdonly str;
            t_font_id font_id;
            void (*click_func)(zcl::t_list<t_title_screen_page_request> *const requests);
        } button;
    } type_data;
};

struct t_title_screen_page_elem_dynamic {
    zcl::t_b8 hovered;
    zcl::t_f32 scale_offs;
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

constexpr zcl::t_f32 k_title_screen_page_button_gap_vertical = 96.0f;

static t_title_screen_page TitleScreenPageCreate(const t_title_screen_page_id id, const zcl::t_v2_i size, zcl::t_arena *const arena) {
    zcl::t_array_mut<t_title_screen_page_elem_static> elem_statics;

    const zcl::t_v2 buttons_center = {
        size.x * 0.5f,
        size.y * 0.575f,
    };

    switch (id) {
        case ek_title_screen_page_id_home: {
            elem_statics = zcl::ArenaPushArray<t_title_screen_page_elem_static>(arena, 3);

            elem_statics[0] = {
                .position = buttons_center + zcl::t_v2{0.0f, -k_title_screen_page_button_gap_vertical},
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Start"),
                        .font_id = ek_font_id_roboto_40,
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
                .position = buttons_center,
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Options"),
                        .font_id = ek_font_id_roboto_40,
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
                .position = buttons_center + zcl::t_v2{0.0f, k_title_screen_page_button_gap_vertical},
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Exit"),
                        .font_id = ek_font_id_roboto_40,
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
            elem_statics = zcl::ArenaPushArray<t_title_screen_page_elem_static>(arena, 1);

            elem_statics[0] = {
                .position = buttons_center,
                .type_id = ek_title_screen_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Back"),
                        .font_id = ek_font_id_roboto_40,
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

t_title_screen_phase *TitleScreenPhaseInit(const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_title_screen_phase>(arena);

    result->page_id = ek_title_screen_page_id_home;

    const auto page_arena_mem = zcl::ArenaPushArray<zcl::t_u8>(arena, zcl::KilobytesToBytes(1));
    result->page_arena = zcl::ArenaCreateWrapping(page_arena_mem);

    result->page = TitleScreenPageCreate(result->page_id, screen_size, result->page_arena);

    return result;
}

t_title_screen_phase_tick_result_id TitleScreenPhaseTick(t_title_screen_phase *const ts, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena) {
    t_title_screen_phase_tick_result_id result = ek_title_screen_phase_tick_result_id_normal;

    const auto cursor_position = zgl::CursorGetPos(input_state);
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
    auto page_requests = zcl::ListCreate<t_title_screen_page_request>(32, temp_arena);

    for (zcl::t_i32 i = 0; i < ts->page.elem_statics.len; i++) {
        const auto elem_static = &ts->page.elem_statics[i];

        const auto elem_dynamic = &ts->page.elem_dynamics[i];
        elem_dynamic->hovered = false;

        switch (elem_static->type_id) {
            case ek_title_screen_page_elem_type_id_button: {
                const auto btn_str_collider = zgl::CalcStrRenderCollider(elem_static->type_data.button.str, *FontGet(assets, elem_static->type_data.button.font_id), elem_static->position, temp_arena, temp_arena, zcl::k_origin_center);

                if (zcl::CheckPointInPoly(btn_str_collider, cursor_position)) {
                    elem_dynamic->hovered = true;

                    if (elem_static->type_data.button.click_func && mouse_button_pressed) {
                        elem_static->type_data.button.click_func(&page_requests);
                    }
                }

                break;
            }

            default: {
                ZCL_UNREACHABLE();
            }
        }

        const zcl::t_f32 scale_offs_targ = elem_dynamic->hovered ? k_title_screen_button_hover_scale_offs : 0.0f;
        elem_dynamic->scale_offs = zcl::Lerp(elem_dynamic->scale_offs, scale_offs_targ, k_title_screen_button_hover_scale_offs_lerp_factor);
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

void TitleScreenPhaseRenderUI(const t_title_screen_phase *const ts, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    // Update page elements.
    for (zcl::t_i32 i = 0; i < ts->page.elem_statics.len; i++) {
        const auto elem_static = &ts->page.elem_statics[i];
        const auto elem_dynamic = &ts->page.elem_dynamics[i];

        switch (elem_static->type_id) {
            case ek_title_screen_page_elem_type_id_button: {
                RenderStrWithOutline(rc, elem_static->type_data.button.str, *FontGet(assets, elem_static->type_data.button.font_id), elem_static->position, elem_dynamic->hovered ? zcl::k_color_yellow : zcl::k_color_white, temp_arena, zcl::k_origin_center, 0.0f, {1.0f + elem_dynamic->scale_offs, 1.0f + elem_dynamic->scale_offs});
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
