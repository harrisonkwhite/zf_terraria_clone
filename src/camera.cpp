#include "camera.h"

struct t_camera {
    zcl::t_v2 position;
    zcl::t_f32 scale;
    zcl::t_f32 lerp_factor;
};

t_camera *CameraCreate(const zcl::t_v2 position, const zcl::t_f32 scale, const zcl::t_f32 lerp_factor, zcl::t_arena *const arena) {
    ZCL_ASSERT(lerp_factor >= 0.0f && lerp_factor <= 1.0f);
    ZCL_ASSERT(scale > 0.0f);

    const auto result = zcl::ArenaPush<t_camera>(arena);
    result->position = position;
    result->scale = scale;
    result->lerp_factor = lerp_factor;

    return result;
}

zcl::t_f32 CameraGetScale(const t_camera *const camera) {
    return camera->scale;
}

void CameraSetScale(t_camera *const camera, const zcl::t_f32 scale) {
    ZCL_ASSERT(scale > 0.0f);
    camera->scale = scale;
}

void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ) {
#if 0
    camera->position = zcl::Lerp(camera->position, pos_targ, camera->lerp_factor);
#endif
    camera->position = pos_targ;
}

zcl::t_rect_f CameraCalcRect(const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    return {
        .x = camera->position.x - (backbuffer_size.x / (2.0f * camera->scale)),
        .y = camera->position.y - (backbuffer_size.y / (2.0f * camera->scale)),
        .width = backbuffer_size.x / camera->scale,
        .height = backbuffer_size.y / camera->scale,
    };
}

zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    ZCL_ASSERT(backbuffer_size.x > 0 && backbuffer_size.y > 0);

    zcl::t_mat4x4 result = zcl::MatrixCreateIdentity();

    result = zcl::MatrixMultiply(result, zcl::MatrixCreateScaled({camera->scale, camera->scale}));

    result = zcl::MatrixMultiply(result, zcl::MatrixCreateTranslated((-camera->position * camera->scale) + (zcl::V2IToF(backbuffer_size) / 2.0f)));

    return result;
}
