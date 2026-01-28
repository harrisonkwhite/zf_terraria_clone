#pragma once


// ============================================================
// @section: External Forward Declarations
// ============================================================

struct t_assets;

// ============================================================


struct t_title_screen;

t_title_screen *TitleScreenInit(zcl::t_arena *const arena);
void TitleScreenTick(t_title_screen *const ts);
void TitleScreenRenderUI(t_title_screen *const ts, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena);
