#include "options.h"

union t_option_value {
    zcl::t_f32 perc;
    zcl::t_b8 toggle;
};

struct t_options {
    zcl::t_static_array<t_option_value, ekm_option_id_cnt> values;
};

t_options *OptionsCreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_options>(arena);
}

zcl::t_f32 OptionsGetPerc(const t_options *const opts, const t_option_id id) {
    ZCL_ASSERT(g_option_metas[id].type_id == ek_option_type_id_perc);
    return opts->values[id].perc;
}

void OptionsSetPerc(t_options *const opts, const t_option_id id, const zcl::t_f32 value) {
    ZCL_ASSERT(g_option_metas[id].type_id == ek_option_type_id_perc);
    ZCL_ASSERT(value >= 0.0f && value <= 1.0f);

    opts->values[id].perc = value;
}

zcl::t_b8 OptionsGetToggle(const t_options *const opts, const t_option_id id) {
    ZCL_ASSERT(g_option_metas[id].type_id == ek_option_type_id_toggle);
    return opts->values[id].toggle;
}

void OptionsSetToggle(t_options *const opts, const t_option_id id, const zcl::t_b8 value) {
    ZCL_ASSERT(g_option_metas[id].type_id == ek_option_type_id_toggle);
    opts->values[id].toggle = value;
}
