#pragma once

struct t_page;

enum t_page_elem_type_id : zcl::t_i32 {
    ek_page_elem_type_id_button,
    ek_page_elem_type_id_slot
};

struct t_page_elem {
    zcl::t_v2 position;

    t_page_elem_type_id type_id;

    union {
        struct {
            zcl::t_str_rdonly str;

            const zgl::t_font *font;

            void (*click_func)(void *const func_data);
            void *click_func_data;
        } button;

        struct {
            zcl::t_v2 size;
        } slot;
    } type_data;
};

t_page *PageCreate(const zcl::t_v2_i size, const zcl::t_array_rdonly<t_page_elem> elems, zcl::t_arena *const arena);
void PageUpdate(t_page *const page, const zcl::t_v2 cursor_position, const zcl::t_b8 mouse_button_pressed, zcl::t_arena *const temp_arena);
void PageRender(const t_page *const page, const zgl::t_rendering_context rendering_context, zcl::t_arena *const temp_arena);
