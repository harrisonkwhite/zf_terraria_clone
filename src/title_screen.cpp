#include "title_screen.h"

#include "assets.h"
#include "ui.h"

enum t_title_screen_page_id : zcl::t_i32 {
    ek_title_screen_page_id_home,
    ek_title_screen_page_id_options
};

enum t_title_screen_request_type_id : zcl::t_i32 {
    ek_title_screen_request_type_id_switch_page,
    ek_title_screen_request_type_id_go_to_world,
    ek_title_screen_request_type_id_exit_game
};

struct t_title_screen_request {
    t_title_screen_request_type_id type_id;

    union {
        struct {
            t_title_screen_page_id page_id;
        } switch_page;
    } type_data;
};

struct t_title_screen_requests {
    zcl::t_list<t_title_screen_request> list;
    zcl::t_arena *arena;
};

struct t_title_screen {
    t_page *page_current;
    t_title_screen_page_id page_current_id;
    zcl::t_arena page_current_arena;
    t_title_screen_requests requests;
};

constexpr zcl::t_f32 k_title_screen_page_button_gap_vertical = 96.0f;

static t_page *TitleScreenPageCreate(const t_title_screen_page_id id, const zcl::t_v2_i size, t_title_screen_requests *const requests, const t_assets *const assets, zcl::t_arena *const arena) {
    static const auto g_request_submitter = [](t_title_screen_requests *const requests, const t_title_screen_request request) {
        zcl::ListAppendDynamic(&requests->list, request, requests->arena);
    };

    switch (id) {
    case ek_title_screen_page_id_home: {
        const auto buttons = zcl::ArenaPushArray<t_page_button>(arena, 3);

        buttons[0] = {
            .position = (zcl::V2IToF(size) / 2.0f) + zcl::t_v2{0.0f, -k_title_screen_page_button_gap_vertical},
            .str = ZCL_STR_LITERAL("Start"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
            .callback_func = [](void *const requests_generic) {
                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);
                g_request_submitter(requests, {.type_id = ek_title_screen_request_type_id_go_to_world});
            },
            .callback_func_data = requests,
        };

        buttons[1] = {
            .position = zcl::V2IToF(size) / 2.0f,
            .str = ZCL_STR_LITERAL("Options"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
            .callback_func = [](void *const requests_generic) {
                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);

                const t_title_screen_request request = {
                    .type_id = ek_title_screen_request_type_id_switch_page,
                    .type_data = {.switch_page = {.page_id = ek_title_screen_page_id_options}},
                };

                g_request_submitter(requests, request);
            },
            .callback_func_data = requests,
        };

        buttons[2] = {
            .position = (zcl::V2IToF(size) / 2.0f) + zcl::t_v2{0.0f, k_title_screen_page_button_gap_vertical},
            .str = ZCL_STR_LITERAL("Exit"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
            .callback_func = [](void *const requests_generic) {
                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);
                g_request_submitter(requests, {.type_id = ek_title_screen_request_type_id_exit_game});
            },
            .callback_func_data = requests,
        };

        return UIPageCreate(size, buttons, arena);
    }

    case ek_title_screen_page_id_options: {
        const auto buttons = zcl::ArenaPushArray<t_page_button>(arena, 1);

        buttons[0] = {
            .position = zcl::V2IToF(size) / 2.0f,
            .str = ZCL_STR_LITERAL("Back"),
            .font = GetFont(assets, ek_font_id_eb_garamond_48),
            .callback_func = [](void *const requests_generic) {
                const auto requests = static_cast<t_title_screen_requests *>(requests_generic);

                const t_title_screen_request request = {
                    .type_id = ek_title_screen_request_type_id_switch_page,
                    .type_data = {.switch_page = {.page_id = ek_title_screen_page_id_home}},
                };

                g_request_submitter(requests, request);
            },
            .callback_func_data = requests,
        };

        return UIPageCreate(size, buttons, arena);
    }

    default:
        ZCL_UNREACHABLE();
    }
}

t_title_screen *TitleScreenInit(const t_assets *const assets, const zgl::t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_title_screen>(arena);
    result->page_current = TitleScreenPageCreate(ek_title_screen_page_id_home, zgl::WindowGetFramebufferSizeCache(platform_ticket), &result->requests, assets, arena);
    result->page_current_id = ek_title_screen_page_id_home;
    result->page_current_arena = zcl::ArenaCreateBlockBased();
    result->requests = {
        .arena = arena,
    };

    return result;
}

t_title_screen_tick_result_id TitleScreenTick(t_title_screen *const ts, const t_assets *const assets, const zgl::t_input_state *const input_state, const zgl::t_platform_ticket_rdonly platform_ticket, zcl::t_arena *const temp_arena) {
    t_title_screen_tick_result_id result = ek_title_screen_tick_result_id_normal;

    UIPageUpdate(ts->page_current, zgl::CursorGetPos(input_state), zgl::MouseButtonCheckPressed(input_state, zgl::ek_mouse_button_code_left), temp_arena);

    for (zcl::t_i32 i = 0; i < ts->requests.list.len; i++) {
        const auto request = &ts->requests.list[i];

        switch (request->type_id) {
        case ek_title_screen_request_type_id_switch_page:
            zcl::ArenaRewind(&ts->page_current_arena);
            ts->page_current = TitleScreenPageCreate(request->type_data.switch_page.page_id, zgl::WindowGetFramebufferSizeCache(platform_ticket), &ts->requests, assets, &ts->page_current_arena);
            ts->page_current_id = request->type_data.switch_page.page_id;
            break;

        case ek_title_screen_request_type_id_go_to_world:
            result = ek_title_screen_tick_result_id_go_to_world;
            break;

        case ek_title_screen_request_type_id_exit_game:
            result = ek_title_screen_tick_result_id_exit_game;
            break;
        }
    }

    zcl::ListClear(&ts->requests.list);

    return result;
}

void TitleScreenRenderUI(const t_title_screen *const ts, const zgl::t_rendering_context rendering_context, const t_assets *const assets, zcl::t_arena *const temp_arena) {
#if 0
    const zcl::t_v2 title_position = zcl::V2IToF(zgl::BackbufferGetSize(rc.gfx_ticket)) / 2.0f;
    zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("Terraria"), *GetFont(assets, ek_font_id_eb_garamond_128), title_position, temp_arena, zcl::k_origin_center);
#endif

    UIPageRender(ts->page_current, rendering_context, temp_arena);
}

void TitleScreenProcessBackbufferResize(t_title_screen *const ts, const zcl::t_v2_i backbuffer_size, const t_assets *const assets) {
    zcl::ArenaRewind(&ts->page_current_arena);
    ts->page_current = TitleScreenPageCreate(ek_title_screen_page_id_home, backbuffer_size, &ts->requests, assets, &ts->page_current_arena);
}
