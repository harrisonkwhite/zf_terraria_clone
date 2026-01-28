#include "title_screen.h"

#include "assets.h"

struct t_title_screen {
};

t_title_screen *TitleScreenInit(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_title_screen>(arena);
}

void TitleScreenTick(t_title_screen *const ts) {
}

void TitleScreenRenderUI(t_title_screen *const ts, const zgl::t_rendering_context rc, const t_assets *const assets, zcl::t_arena *const temp_arena) {
    zgl::RendererPassBegin(rc, zgl::BackbufferGetSize(rc.gfx_ticket), zcl::MatrixCreateIdentity(), true, zcl::k_color_red);

    const zcl::t_v2 title_position = zcl::V2IToF(zgl::BackbufferGetSize(rc.gfx_ticket)) / 2.0f;
    // zgl::RendererSubmitTexture(rc, GetFont(assets, ek_font_id_eb_garamond_128)->atlas_textures[0], {});
    zgl::RendererSubmitStr(rc, ZCL_STR_LITERAL("Terraria"), *GetFont(assets, ek_font_id_eb_garamond_128), title_position, temp_arena, zcl::k_origin_center);

    zgl::RendererPassEnd(rc);
}
