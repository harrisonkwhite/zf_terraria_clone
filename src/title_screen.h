#pragma once

struct t_title_screen;

t_title_screen *TitleScreenInit(zcl::t_arena *const arena);
void TitleScreenTick(t_title_screen *const ts);
void TitleScreenRender(t_title_screen *const ts);
