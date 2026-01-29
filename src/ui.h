#pragma once

struct t_page;

struct t_page_button {
    zcl::t_v2 pos;
    zcl::t_str_rdonly str;
};

t_page *PageCreate(const zcl::t_v2_i size, const zcl::t_array_rdonly<t_page_button> buttons, zcl::t_arena *const arena);
