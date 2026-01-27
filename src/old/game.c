#include "game.h"

#include <stdio.h>
#include "assets.h"

const s_setting g_settings[] = {
    [ek_setting_smooth_camera] = {
        .type = ek_setting_type_toggle,
        .name = "Smooth Camera"
    },
    [ek_setting_volume] = {
        .type = ek_setting_type_perc,
        .name = "Volume"
    }
};

STATIC_ARRAY_LEN_CHECK(g_settings, eks_setting_cnt);

static const t_settings g_settings_default = {
    [ek_setting_smooth_camera] = 1,
    [ek_setting_volume] = 100
};

static bool LoadSettingsFromFile(t_settings* const settings) {
    assert(IS_ZERO(*settings));

    FILE* const fs = fopen(SETTINGS_FILENAME, "rb");

    if (!fs) {
        return false;
    }

    const t_s32 read = fread(settings, 1, sizeof(*settings), fs);

    fclose(fs);

    if (read < sizeof(*settings)) {
        ZERO_OUT(*settings);
        return false;
    }

    return true;
}

static void LoadSettings(t_settings* const settings) {
    assert(IS_ZERO(*settings));

    if (!LoadSettingsFromFile(settings)) {
        // Failed to load settings file, so load defaults.
        memcpy(settings, g_settings_default, sizeof(t_settings));
    }
}

static bool WriteSettingsToFile(t_settings* const settings) {
    FILE* const fs = fopen(SETTINGS_FILENAME, "wb");

    if (!fs) {
        return false;
    }

    const t_s32 written = fwrite(settings, 1, sizeof(*settings), fs);

    fclose(fs);

    if (written != sizeof(*settings)) {
        return false;
    }

    return true;
}

bool InitGame(const s_game_init_context* const zfw_context) {
    s_game* const game = zfw_context->dev_mem;

    if (!InitTextureGroup(&game->textures, eks_texture_cnt, GenTextureRGBA, zfw_context->perm_mem_arena, zfw_context->gl_res_arena, zfw_context->temp_mem_arena)) {
        return false;
    }

    if (!InitFontGroupFromFiles(&game->fonts, (s_char_array_view_array_view)ARRAY_FROM_STATIC(g_font_file_paths), zfw_context->perm_mem_arena, zfw_context->gl_res_arena, zfw_context->temp_mem_arena)) {
        return false;
    }

    if (!InitShaderProgGroup(&game->shader_progs, (s_shader_prog_gen_info_array_view)ARRAY_FROM_STATIC(g_shader_prog_gen_infos), zfw_context->gl_res_arena, zfw_context->temp_mem_arena)) {
        return false;
    }

    if (!InitSurface(&game->global_surf, zfw_context->window_state.size, zfw_context->gl_res_arena)) {
        return false;
    }

    if (!InitSurface(&game->temp_surf, zfw_context->window_state.size, zfw_context->gl_res_arena)) {
        return false;
    }

    LoadSettings(&game->settings);

    if (!InitTitleScreen(&game->title_screen, zfw_context->temp_mem_arena)) {
        return false;
    }

    return true;
}

e_game_tick_result GameTick(const s_game_tick_context* const zfw_context) {
    s_game* const game = zfw_context->dev_mem;

    if (game->in_world) {
        if (!WorldTick(&game->world, &game->settings, zfw_context)) {
            return ek_game_tick_result_error;
        }
    } else {
        const s_title_screen_tick_result tick_res = TitleScreenTick(&game->title_screen, &game->settings, zfw_context, &game->fonts);

        switch (tick_res.type) {
            case ek_title_screen_tick_result_type_normal:
                break;

            case ek_title_screen_tick_result_type_error:
                return ek_game_tick_result_error;

            case ek_title_screen_tick_result_type_load_world:
                ZERO_OUT(game->title_screen);

                if (!InitWorld(&game->world, &tick_res.world_filename, zfw_context->window_state.size, zfw_context->temp_mem_arena)) {
                    return ek_game_tick_result_error;
                }

                game->in_world = true;

                break;

            case ek_title_screen_tick_result_type_exit:
                return ek_game_tick_result_exit;
        }
    }

    return ek_game_tick_result_normal;
}

static inline s_matrix_4x4 UIViewMatrix(const s_v2_s32 window_size) {
    s_matrix_4x4 mat = IdentityMatrix4x4();
    ScaleMatrix4x4(&mat, UIScale(window_size));
    return mat;
}

bool RenderGame(const s_game_render_context* const zfw_context) {
    s_game* const game = zfw_context->dev_mem;

    const s_rendering_context* const rc = &zfw_context->rendering_context;

    if (!V2S32sEqual(game->global_surf.size, rc->window_size)) {
        if (!ResizeSurface(&game->global_surf, rc->window_size)) {
            return false;
        }
    }

    if (!V2S32sEqual(game->temp_surf.size, rc->window_size)) {
        if (!ResizeSurface(&game->temp_surf, rc->window_size)) {
            return false;
        }
    }

    const s_matrix_4x4 ui_view_matrix = UIViewMatrix(rc->window_size);

    SetSurface(rc, &game->global_surf);

    Clear(rc, BG_COLOR);

    if (game->in_world) {
        if (!RenderWorld(&game->world, rc, &game->textures, &game->temp_surf, zfw_context->temp_mem_arena)) {
            return false;
        }

        SetViewMatrix(rc, &ui_view_matrix);

        if (!RenderWorldUI(&game->world, zfw_context, &game->textures, &game->fonts)) {
            return false;
        }
    } else {
        SetViewMatrix(rc, &ui_view_matrix);

        if (!RenderTitleScreen(&game->title_screen, rc, &game->settings, &game->textures, &game->fonts, zfw_context->temp_mem_arena)) {
            return false;
        }
    }

    // Render the mouse.
    const s_v2 mouse_ui_pos = DisplayToUIPos(zfw_context->mouse_pos, rc->window_size);
    RenderSprite(rc, ek_sprite_mouse, &game->textures, mouse_ui_pos, (s_v2){0.5f, 0.5f}, (s_v2){1.0f, 1.0f}, 0.0f, WHITE);

    UnsetSurface(rc);

    SetSurfaceShaderProg(rc, &rc->basis->builtin_shader_progs, ek_builtin_shader_prog_surface_default);
    RenderSurface(rc, &game->global_surf, (s_v2){0}, V2_ONE, false);

    return true;
}

void CleanGame(void* const dev_mem) {
    s_game* const game = dev_mem;

    if (game->in_world) {
        CleanWorld(&game->world);
    }

    WriteSettingsToFile(&game->settings);
}
