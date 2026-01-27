#include "title_screen.h"

struct t_title_screen {
};

t_title_screen *TitleScreenInit(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_title_screen>(arena);
}

void TitleScreenTick(t_title_screen *const ts) {
}

void TitleScreenRender(t_title_screen *const ts, const zgl::t_rendering_context rc, const t_assets *const assets) {
}
