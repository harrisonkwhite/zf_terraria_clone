#include "options.h"

enum t_option_value_type_id : zcl::t_i32 {
    ek_option_value_type_id_b8,
    ek_option_value_type_id_f32,
};

struct t_option_value_set {
    zcl::t_array_rdonly<zcl::t_str_rdonly> names;

    t_option_value_type_id value_type_id;

    union {
        zcl::t_array_rdonly<zcl::t_b8> b8s;
        zcl::t_array_rdonly<zcl::t_f32> f32s;
    } value_type_data;
};

struct t_options {
    zcl::t_static_array<t_option_value_set, ekm_option_id_cnt> value_sets;
    zcl::t_static_array<zcl::t_i32, ekm_option_id_cnt> value_set_indexes;
};

t_options *OptionsCreate(zcl::t_arena *const arena) {
    return zcl::ArenaPush<t_options>(arena);
}

void OptionRegisterValueSetB8(t_options *const opts, const t_option_id opt_id, const zcl::t_array_rdonly<zcl::t_str_rdonly> set_names, const zcl::t_array_rdonly<zcl::t_b8> set_b8s) {
    ZCL_ASSERT(set_names.len == set_b8s.len);

    opts->value_sets[opt_id].names = set_names;
    opts->value_sets[opt_id].value_type_id = ek_option_value_type_id_b8;
    opts->value_sets[opt_id].value_type_data.b8s = set_b8s;
}

void OptionRegisterValueSetF32(t_options *const opts, const t_option_id opt_id, const zcl::t_array_rdonly<zcl::t_str_rdonly> set_names, const zcl::t_array_rdonly<zcl::t_f32> set_f32s) {
    ZCL_ASSERT(set_names.len == set_f32s.len);

    opts->value_sets[opt_id].names = set_names;
    opts->value_sets[opt_id].value_type_id = ek_option_value_type_id_f32;
    opts->value_sets[opt_id].value_type_data.f32s = set_f32s;
}

zcl::t_i32 OptionGetValueIndex(const t_options *const opts, const t_option_id id) {
    return opts->value_set_indexes[id];
}

void OptionSetValueIndex(t_options *const opts, const t_option_id id, const zcl::t_i32 index) {
    ZCL_ASSERT(index >= 0 && index < OptionGetValueCount(opts, id));
    opts->value_set_indexes[id] = index;
}

zcl::t_i32 OptionGetValueCount(const t_options *const opts, const t_option_id id) {
    return opts->value_sets[id].names.len;
}

zcl::t_str_rdonly OptionGetValueName(const t_options *const opts, const t_option_id id) {
    return opts->value_sets[id].names[opts->value_set_indexes[id]];
}

zcl::t_b8 OptionGetValueB8(const t_options *const opts, const t_option_id id) {
    return opts->value_sets[id].value_type_data.b8s[opts->value_set_indexes[id]];
}

zcl::t_f32 OptionGetValueF32(const t_options *const opts, const t_option_id id) {
    return opts->value_sets[id].value_type_data.f32s[opts->value_set_indexes[id]];
}
