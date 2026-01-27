#include "game.h"

#include <stdio.h>

#define PAGE_ELEM_COMMON_FONT ek_font_eb_garamond_32
#define PAGE_ELEM_GAP 48.0f
#define PAGE_ELEM_COMMON_PADDING 28.0f
#define PAGE_ELEM_STR_BUF_SIZE 32
static_assert(WORLD_NAME_LEN_LIMIT <= PAGE_ELEM_STR_BUF_SIZE - 1, "A page element must be able to represent a maximum-length world name!");

typedef struct {
    s_title_screen* ts;
    t_settings* settings;
    s_title_screen_tick_result* tick_result;
    s_mem_arena* temp_mem_arena;
} s_page_feature_button_click_data;

typedef bool (*t_page_feature_button_click_func)(const t_s32 index, void* const data);

typedef struct {
    char str[PAGE_ELEM_STR_BUF_SIZE];
    t_r32 padding_top;
    t_r32 padding_bottom;
    e_font font;

    bool button;
    bool button_inactive;
    t_page_feature_button_click_func button_click_func;
} s_page_feature;

DEF_ARRAY_TYPE(s_page_feature, page_feature, PageFeature);

static bool LoadWorldFilenames(t_world_filenames* const filenames, s_mem_arena* const temp_mem_arena) {
    assert(IS_ZERO(*filenames));

    s_filename_buf_array local_filename_bufs = {0};

    if (!LoadDirFilenames(&local_filename_bufs, temp_mem_arena, (s_char_array_view)ARRAY_FROM_STATIC("."))) {
        return false;
    }

    t_s32 next_index = 0;

    for (t_s32 i = 0; i < local_filename_bufs.elem_cnt; i++) {
        t_filename_buf* const buf = FilenameBufElem(local_filename_bufs, i);
        const s_char_array_view local_filename = StrViewFromRawTerminated(*buf);

        if (DoesFilenameHaveExt(local_filename, (s_char_array_view)ARRAY_FROM_STATIC(WORLD_FILENAME_EXT))) {
            // TODO: Make sure the world filename isn't too long!
            strncpy(*STATIC_ARRAY_ELEM(*filenames, next_index), *buf, sizeof((*filenames)[next_index]));
            next_index++;

            if (next_index == WORLD_LIMIT) {
                break;
            }
        }
    }

    return true;
}

static t_s32 WorldCnt(const t_world_filenames* const filenames) {
    t_s32 cnt = 0;

    for (t_s32 i = 0; i < WORLD_LIMIT; i++) {
        if ((*filenames)[i][0]) {
            cnt++;
        }
    }

    return cnt;
}

static bool HomePagePlayButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    return true;
}

static bool HomePageSettingsButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_settings;
    return true;
}

static bool HomePageExitButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;

    *data->tick_result = (s_title_screen_tick_result){
        .type = ek_title_screen_tick_result_type_exit
    };

    return true;
}

static bool WorldsPageWorldButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;

    const t_s32 world_index = index - 1;
    assert(world_index >= 0 && world_index < WORLD_LIMIT);
    assert(data->ts->world_filenames_cache[world_index][0]);

    *data->tick_result = (s_title_screen_tick_result){
        .type = ek_title_screen_tick_result_type_load_world
    };

    memcpy(data->tick_result->world_filename, data->ts->world_filenames_cache[world_index], sizeof(data->ts->world_filenames_cache[world_index]));

    return true;
}

static bool WorldsPageCreateButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_new_world;
    assert(IS_ZERO(data->ts->new_world_name_buf));
    return true;
}

static bool WorldsPageBackButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static bool NewWorldPageAcceptButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;

    data->ts->page = ek_title_screen_page_worlds;

    t_world_filename filename = {0};
    snprintf(filename, sizeof(filename), "%s%s", data->ts->new_world_name_buf, WORLD_FILENAME_EXT);

    {
        s_world_core* const world_core = PushToMemArena(data->temp_mem_arena, sizeof(s_world_core), ALIGN_OF(s_world_core));

        if (!world_core) {
            return false;
        }

        GenWorld(world_core);

        if (!WriteWorldCoreToFile(world_core, &filename)) {
            return false;
        }
    }

    ZERO_OUT(data->ts->new_world_name_buf);

    ZERO_OUT(data->ts->world_filenames_cache);
    return LoadWorldFilenames(&data->ts->world_filenames_cache, data->temp_mem_arena);
}

static bool NewWorldPageBackButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    ZERO_OUT(data->ts->new_world_name_buf);
    return true;
}

static bool SettingsPageSettingButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;

    assert(index >= 0 && index < eks_setting_cnt);

    t_s32 limit = 0, inc = 1;

    switch (g_settings[index].type) {
        case ek_setting_type_toggle:
            limit = 1;
            break;

        case ek_setting_type_perc:
            limit = 100;
            inc = 5;
            break;
    }

    if ((*data->settings)[index] == limit) {
        (*data->settings)[index] = 0;
    } else {
        (*data->settings)[index] += inc;
    }

    return true;
}

static bool SettingsPageBackButtonClick(const t_s32 index, void* const data_generic) {
    const s_page_feature_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static bool WARN_UNUSED_RESULT GenPageFeatures(s_page_feature_array* const feats, s_mem_arena* const mem_arena, const e_title_screen_page page, const t_world_filenames* const world_filenames, const t_world_name_buf* const new_world_name_buf, const t_settings* const settings) {
    assert(IS_ZERO(*feats));

    t_s32 feat_cnt = 0;

    switch (page) {
        case ek_title_screen_page_home: feat_cnt = 4; break;
        case ek_title_screen_page_worlds: feat_cnt = 1 + WORLD_LIMIT + 2; break;
        case ek_title_screen_page_new_world: feat_cnt = 4; break;
        case ek_title_screen_page_settings: feat_cnt = eks_setting_cnt + 1; break;

        default:
            assert(false && "Unhandled page case!");
            break;
    }

    *feats = PushPageFeatureArrayToMemArena(mem_arena, feat_cnt);

    if (IS_ZERO(*feats)) {
        return false;
    }

    switch (page) {
        case ek_title_screen_page_home:
            for (t_s32 i = 0; i < feat_cnt; i++) {
                switch (i) {
                    case 0:
                        *PageFeatureElem(*feats, i) = (s_page_feature){
                            .str = GAME_TITLE,
                            .font = ek_font_eb_garamond_80,
                            .padding_bottom = 48.0f
                        };

                        break;

                    case 1:
                        *PageFeatureElem(*feats, i) = (s_page_feature){
                            .str = "Play",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePagePlayButtonClick
                        };

                        break;

                    case 2:
                        *PageFeatureElem(*feats, i) = (s_page_feature){
                            .str = "Settings",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePageSettingsButtonClick
                        };

                        break;

                    case 3:
                        *PageFeatureElem(*feats, i) = (s_page_feature){
                            .str = "Exit",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePageExitButtonClick
                        };

                        break;

                    default:
                        assert(false && "Unhandled page feature case!");
                        break;
                }
            }

            break;

        case ek_title_screen_page_worlds:
            *PageFeatureElem(*feats, 0) = (s_page_feature){
                .str = "Select a World",
                .font = ek_font_eb_garamond_48,
                .padding_bottom = PAGE_ELEM_COMMON_PADDING
            }; 

            for (t_s32 i = 0; i < WORLD_LIMIT; i++) {
                s_page_feature* const feat = PageFeatureElem(*feats, 1 + i);

                *feat = (s_page_feature){
                    .font = PAGE_ELEM_COMMON_FONT,
                    .button = true,
                    .button_click_func = WorldsPageWorldButtonClick
                };

                if (!(*world_filenames)[i][0]) {
                    snprintf(feat->str, sizeof(feat->str), "Empty");
                    feat->button_inactive = true;
                } else {
                    snprintf(feat->str, sizeof(feat->str), "%s", (*world_filenames)[i]);
                }
            }

            *PageFeatureElem(*feats, 1 + WORLD_LIMIT) = (s_page_feature){
                .str = "Create New World",
                .font = PAGE_ELEM_COMMON_FONT,
                .padding_top = PAGE_ELEM_COMMON_PADDING,
                .button = true,
                .button_inactive = WorldCnt(world_filenames) == WORLD_LIMIT,
                .button_click_func = WorldsPageCreateButtonClick
            };

            *PageFeatureElem(*feats, 1 + WORLD_LIMIT + 1) = (s_page_feature){
                .str = "Back",
                .font = PAGE_ELEM_COMMON_FONT,
                .button = true,
                .button_click_func = WorldsPageBackButtonClick
            };

            break;

        case ek_title_screen_page_new_world:
            *PageFeatureElem(*feats, 0) = (s_page_feature){
                .str = "Enter World Name",
                .font = ek_font_eb_garamond_48,
                .padding_bottom = PAGE_ELEM_COMMON_PADDING
            };

            *PageFeatureElem(*feats, 1) = (s_page_feature){
                .font = PAGE_ELEM_COMMON_FONT
            };

            snprintf(PageFeatureElem(*feats, 1)->str, sizeof(PageFeatureElem(*feats, 1)->str), "%s", *new_world_name_buf);

            *PageFeatureElem(*feats, 2) = (s_page_feature){
                .str = "Accept",
                .padding_top = PAGE_ELEM_COMMON_PADDING,
                .font = PAGE_ELEM_COMMON_FONT,
                .button = true,
                .button_inactive = !(*new_world_name_buf)[0],
                .button_click_func = NewWorldPageAcceptButtonClick
            };

            *PageFeatureElem(*feats, 3) = (s_page_feature){
                .str = "Back",
                .font = PAGE_ELEM_COMMON_FONT,
                .button = true,
                .button_click_func = NewWorldPageBackButtonClick
            };

            break;

        case ek_title_screen_page_settings:
            for (t_s32 i = 0; i < eks_setting_cnt; i++) {
                s_page_feature* const feat = PageFeatureElem(*feats, i);

                *feat = (s_page_feature){
                    .font = PAGE_ELEM_COMMON_FONT,

                    .button = true,
                    .button_click_func = SettingsPageSettingButtonClick
                };

                switch ((e_setting_type)g_settings[i].type) {
                    case ek_setting_type_toggle:
                        if (SettingToggle(settings, i)) {
                            snprintf(feat->str, sizeof(feat->str), "%s: Enabled", g_settings[i].name.buf_raw);
                        } else {
                            snprintf(feat->str, sizeof(feat->str), "%s: Disabled", g_settings[i].name.buf_raw);
                        }

                        break;

                    case ek_setting_type_perc:
                        snprintf(feat->str, sizeof(feat->str), "%s: %d%%", g_settings[i].name.buf_raw, (t_s32)(*settings)[i]);
                        break;
                }

            }

            *PageFeatureElem(*feats, eks_setting_cnt) = (s_page_feature){
                .str = "Back",
                .font = PAGE_ELEM_COMMON_FONT,
                .padding_top = PAGE_ELEM_COMMON_PADDING,
                .button = true,
                .button_click_func = SettingsPageBackButtonClick
            };

            break;

        default:
            assert(false && "Unhandled page case!");
            break;
    }

    return true;
}

static bool WARN_UNUSED_RESULT GenPageFeaturePositions(s_v2_array* const positions, s_mem_arena* const mem_arena, const s_page_feature_array_view feats, const s_v2_s32 ui_size) {
    *positions = PushV2ArrayToMemArena(mem_arena, feats.elem_cnt);

    if (IS_ZERO(*positions)) {
        return false;
    }

    t_r32 span_y = 0.0f;

    for (t_s32 i = 0; i < feats.elem_cnt; i++) {
        const s_page_feature* const feat = PageFeatureElemView(feats, i);

        span_y += feat->padding_top;

        *V2Elem(*positions, i) = (s_v2){
            ui_size.x / 2.0f,
            span_y
        };

        span_y += feat->padding_bottom;

        if (i < feats.elem_cnt - 1) {
            span_y += PAGE_ELEM_GAP;
        }
    }

    // Shift everything vertically to the middle.
    for (t_s32 i = 0; i < feats.elem_cnt; i++) {
        V2Elem(*positions, i)->y += (ui_size.y - span_y) / 2.0f;
    }

    return true;
}

bool InitTitleScreen(s_title_screen* const ts, s_mem_arena* const temp_mem_arena) {
    assert(IS_ZERO(*ts));

    ts->page_btn_feat_hovered_index = -1;

    if (!LoadWorldFilenames(&ts->world_filenames_cache, temp_mem_arena)) {
        return false;
    }

    return true;
}

static bool WARN_UNUSED_RESULT LoadIndexOfFirstHoveredButtonPageElem(t_s32* const index, const s_v2 cursor_ui_pos, const s_page_feature_array_view feats, const s_v2_array_view feat_positions, const s_font_group* const fonts, s_mem_arena* const temp_mem_arena) {
    *index = -1;

    for (t_s32 i = 0; i < feats.elem_cnt; i++) {
        const s_page_feature* const feat = PageFeatureElemView(feats, i);

        if (!feat->button || feat->button_inactive) {
            continue;
        }

        s_rect collider = {0};

        if (!GenStrCollider(&collider, (s_char_array_view)ARRAY_FROM_STATIC(feat->str), fonts, feat->font, *V2ElemView(feat_positions, i), ALIGNMENT_CENTER, temp_mem_arena)) {
            return false;
        }

        if (IsPointInRect(cursor_ui_pos, collider)) {
            *index = i;
            break;
        }
    }

    return true;
}

s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, t_settings* const settings, const s_game_tick_context* const zfw_context, const s_font_group* const fonts) {
    s_title_screen_tick_result result = {0};

    s_page_feature_array page_feats = {0};

    if (!GenPageFeatures(&page_feats, zfw_context->temp_mem_arena, ts->page, &ts->world_filenames_cache, &ts->new_world_name_buf, settings)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    const s_v2_s32 ui_size = UISize(zfw_context->window_state.size);

    s_v2_array page_feat_positions = {0};

    if (!GenPageFeaturePositions(&page_feat_positions, zfw_context->temp_mem_arena, PageFeatureArrayView(page_feats), ui_size)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    const s_v2 cursor_ui_pos = DisplayToUIPos(zfw_context->input_context.state->mouse_pos, zfw_context->window_state.size);

    if (!LoadIndexOfFirstHoveredButtonPageElem(&ts->page_btn_feat_hovered_index, cursor_ui_pos, PageFeatureArrayView(page_feats), V2ArrayView(page_feat_positions), fonts, zfw_context->temp_mem_arena)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    if (ts->page_btn_feat_hovered_index != -1) {
        if (IsMouseButtonPressed(&zfw_context->input_context, ek_mouse_button_code_left)) {
            s_page_feature* const feat = PageFeatureElem(page_feats, ts->page_btn_feat_hovered_index);

            if (feat->button_click_func) {
                s_page_feature_button_click_data btn_click_data = {
                    .ts = ts,
                    .settings = settings,
                    .tick_result = &result,
                    .temp_mem_arena = zfw_context->temp_mem_arena
                };

                if (!feat->button_click_func(ts->page_btn_feat_hovered_index, &btn_click_data)) {
                    return (s_title_screen_tick_result){
                        .type = ek_title_screen_tick_result_type_error
                    };
                }
            } else {
                assert(false && "Button click function not set!");
            }
        }
    }

    if (ts->page == ek_title_screen_page_new_world) {
        t_s32 nw_name_buf_index = 0;

        while (nw_name_buf_index < sizeof(ts->new_world_name_buf) && ts->new_world_name_buf[nw_name_buf_index]) {
            nw_name_buf_index++;
        }

        for (size_t i = 0; i < sizeof(zfw_context->input_context.events->unicode_buf) && nw_name_buf_index < sizeof(ts->new_world_name_buf) - 1; i++) {
            const char c = zfw_context->input_context.events->unicode_buf[i];

            if (!c) {
                break;
            }

            ts->new_world_name_buf[nw_name_buf_index] = c;
            nw_name_buf_index++;
        }

        if (nw_name_buf_index > 0) {
            if (IsKeyPressed(&zfw_context->input_context, ek_key_code_backspace)) {
                nw_name_buf_index--;
                ts->new_world_name_buf[nw_name_buf_index] = '\0';
            }
        }
    }

    return result;
}

bool RenderTitleScreen(const s_title_screen* const ts, const s_rendering_context* const rendering_context, const t_settings* const settings, const s_texture_group* const textures, const s_font_group* const fonts, s_mem_arena* const temp_mem_arena) {
    s_page_feature_array page_feats = {0};

    if (!GenPageFeatures(&page_feats, temp_mem_arena, ts->page, &ts->world_filenames_cache, &ts->new_world_name_buf, settings)) {
        return false;
    }

    s_v2_array page_feat_positions = {0};

    if (!GenPageFeaturePositions(&page_feat_positions, temp_mem_arena, PageFeatureArrayView(page_feats), UISize(rendering_context->window_size))) {
        return false;
    }

    for (t_s32 i = 0; i < page_feats.elem_cnt; i++) {
        const s_page_feature* const elem = PageFeatureElem(page_feats, i);

        u_v4 color = WHITE;

        if (elem->button) {
            if (elem->button_inactive) {
                color = DARK_GRAY;
            } else if (ts->page_btn_feat_hovered_index == i) {
                color = YELLOW;
            }
        }

        if (!RenderStr(rendering_context, (s_char_array_view)ARRAY_FROM_STATIC(elem->str), fonts, elem->font, *V2Elem(page_feat_positions, i), ALIGNMENT_CENTER, color, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}
