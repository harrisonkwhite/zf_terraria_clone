#include "title_screen_phase.h"

#include "assets.h"
#include "ui_helpers.h"

constexpr zcl::t_f32 k_title_screen_logo_wave_rot_acc = 0.01f;
constexpr zcl::t_f32 k_title_screen_logo_wave_rot_mult = 0.01f * zcl::k_pi;
constexpr zcl::t_f32 k_title_screen_logo_wave_scale_offs_acc = 0.015f;
constexpr zcl::t_f32 k_title_screen_logo_wave_scale_offs_mult = 0.04f;

enum t_title_screen_page_id : zcl::t_i32 {
    ek_title_screen_page_id_home,
    ek_title_screen_page_id_options
};

struct t_title_screen_phase {
    zcl::t_f32 logo_wave_rot;
    zcl::t_f32 logo_wave_scale_offs;

    t_title_screen_page_id page_current_id;
};

#if 0
constexpr zcl::t_f32 k_title_screen_page_button_gap_vertical = 96.0f;

static t_page *TitleScreenPageCreate(const t_title_screen_page_id id, const zcl::t_v2_i size, t_title_screen_page_elem_click_func_data *const elem_click_func_data, zcl::t_arena *const arena) {
    static const auto g_request_submitter = [](t_title_screen_requests *const requests, const t_title_screen_request request) {
        zcl::ListAppendDynamic(&requests->list, request, requests->arena);
    };

    const zcl::t_v2 buttons_center = {
        size.x * 0.5f,
        size.y * 0.575f,
    };

    switch (id) {
        case ek_title_screen_page_id_home: {
            const auto elems = zcl::ArenaPushArray<t_page_elem_static>(arena, 3);

            elems[0] = {
                .position = buttons_center + zcl::t_v2{0.0f, -k_title_screen_page_button_gap_vertical},
                .type_id = ek_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Start"),
                        .font = FontGet(elem_click_func_data->assets, ek_font_id_roboto_40),
                        .click_func =
                            [](void *const requests_generic) {
                                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);
                                g_request_submitter(requests, {.type_id = ek_title_screen_request_type_id_go_to_world});
                            },
                        .click_func_data = elem_click_func_data->requests,
                    },
                },
            };

            elems[1] = {
                .position = buttons_center,
                .type_id = ek_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Options"),
                        .font = FontGet(elem_click_func_data->assets, ek_font_id_roboto_40),
                        .click_func =
                            [](void *const requests_generic) {
                                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);

                                const t_title_screen_request request = {
                                    .type_id = ek_title_screen_request_type_id_switch_page,
                                    .type_data = {.switch_page = {.page_id = ek_title_screen_page_id_options}},
                                };

                                g_request_submitter(requests, request);
                            },
                        .click_func_data = elem_click_func_data->requests,
                    },
                },
            };

            elems[2] = {
                .position = buttons_center + zcl::t_v2{0.0f, k_title_screen_page_button_gap_vertical},
                .type_id = ek_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Exit"),
                        .font = FontGet(elem_click_func_data->assets, ek_font_id_roboto_40),
                        .click_func =
                            [](void *const requests_generic) {
                                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);
                                g_request_submitter(requests, {.type_id = ek_title_screen_request_type_id_exit_game});
                            },
                        .click_func_data = elem_click_func_data->requests,
                    },
                },
            };

            return PageCreate(size, elems, arena);
        }

        case ek_title_screen_page_id_options: {
            const auto elems = zcl::ArenaPushArray<t_page_elem_static>(arena, 1);

            elems[0] = {
                .position = buttons_center,
                .type_id = ek_page_elem_type_id_button,
                .type_data = {
                    .button = {
                        .str = ZCL_STR_LITERAL("Back"),
                        .font = FontGet(elem_click_func_data->assets, ek_font_id_roboto_40),
                        .click_func =
                            [](void *const requests_generic) {
                                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);

                                const t_title_screen_request request = {
                                    .type_id = ek_title_screen_request_type_id_switch_page,
                                    .type_data = {.switch_page = {.page_id = ek_title_screen_page_id_home}},
                                };

                                g_request_submitter(requests, request);
                            },
                        .click_func_data = elem_click_func_data->requests,
                    },
                },
            };

            return PageCreate(size, elems, arena);
        }

        default:
            ZCL_UNREACHABLE();
    }
}
#endif

t_title_screen_phase *TitleScreenPhaseInit(const t_assets *const assets, const zcl::t_v2_i screen_size, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_title_screen_phase>(arena);
    result->page_current_id = ek_title_screen_page_id_home;
#if 0
    result->page_current = TitleScreenPageCreate(ek_title_screen_page_id_home, screen_size, &result->requests, assets, arena);
    result->page_current_arena = zcl::ArenaCreateBlockBased();
    result->requests = {.arena = arena};
#endif

    return result;
}

t_title_screen_phase_tick_result_id TitleScreenPhaseTick(t_title_screen_phase *const ts, const t_assets *const assets, const zgl::t_input_state *const input_state, const zcl::t_v2_i screen_size, const zgl::t_audio_ticket_mut audio_ticket, zcl::t_arena *const temp_arena) {
    t_title_screen_phase_tick_result_id result = ek_title_screen_phase_tick_result_id_normal;

    ts->logo_wave_rot += k_title_screen_logo_wave_rot_acc;

    while (ts->logo_wave_rot > 2.0f * zcl::k_pi) {
        ts->logo_wave_rot -= 2.0f * zcl::k_pi;
    }

    ts->logo_wave_scale_offs += k_title_screen_logo_wave_scale_offs_acc;

    while (ts->logo_wave_scale_offs > 2.0f * zcl::k_pi) {
        ts->logo_wave_scale_offs -= 2.0f * zcl::k_pi;
    }

#if 0
    PageUpdate(ts->page_current, zgl::CursorGetPos(input_state), zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left), temp_arena);

    for (zcl::t_i32 i = 0; i < ts->requests.list.len; i++) {
        const auto request = &ts->requests.list[i];

        switch (request->type_id) {
            case ek_title_screen_request_type_id_switch_page: {
                zcl::ArenaRewind(ts->page_current_arena);
                ts->page_current = TitleScreenPageCreate(request->type_data.switch_page.page_id, screen_size, &ts->requests, assets, ts->page_current_arena);
                ts->page_current_id = request->type_data.switch_page.page_id;
                break;
            }

            case ek_title_screen_request_type_id_go_to_world: {
                result = ek_title_screen_phase_tick_result_id_go_to_world;
                break;
            }

            case ek_title_screen_request_type_id_exit_game: {
                result = ek_title_screen_phase_tick_result_id_exit_game;
                break;
            }
        }
    }

    zcl::ListClear(&ts->requests.list);
#endif

    return result;
}

void TitleScreenPhaseRenderUI(const t_title_screen_phase *const ts, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    const zcl::t_v2 logo_position = {rc.screen_size.x * 0.5f, rc.screen_size.y * 0.2f};
    const zcl::t_f32 logo_rot = zcl::Sin(ts->logo_wave_rot) * k_title_screen_logo_wave_rot_mult;
    const zcl::t_f32 logo_scale_offs = zcl::Sin(ts->logo_wave_scale_offs) * k_title_screen_logo_wave_scale_offs_mult;
    RenderStrWithOutline(rc, ZCL_STR_LITERAL("Terraria"), *FontGet(assets, ek_font_id_roboto_184), logo_position, zcl::k_color_white, temp_arena, zcl::k_origin_center, logo_rot, {1.0f - k_title_screen_logo_wave_scale_offs_mult + logo_scale_offs, 1.0f - k_title_screen_logo_wave_scale_offs_mult + logo_scale_offs});

#if 0
    PageRender(ts->page_current, rc, temp_arena);
#endif
}

void TitleScreenPhaseProcessScreenResize(t_title_screen_phase *const ts, const zcl::t_v2_i screen_size, const t_assets *const assets) {
#if 0
    zcl::ArenaRewind(ts->page_current_arena);
    ts->page_current = TitleScreenPageCreate(ek_title_screen_page_id_home, screen_size, &ts->requests, assets, ts->page_current_arena);
#endif
}
