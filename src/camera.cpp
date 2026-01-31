#include "camera.h"

struct t_camera {
    zcl::t_v2 position;
    zcl::t_f32 lerp_factor;
};

t_camera *CameraCreate(const zcl::t_v2 position, const zcl::t_f32 lerp_factor, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_camera>(arena);
    result->position = position;
    result->lerp_factor = lerp_factor;

    return result;
}

void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ) {
    camera->position = zcl::Lerp(camera->position, pos_targ, camera->lerp_factor);
}

zcl::t_f32 CameraCalcScale(const zcl::t_v2_i backbuffer_size) {
    return backbuffer_size.x > 1600 || backbuffer_size.y > 900 ? 1.0f : 1.0f;
}

zcl::t_rect_f CameraCalcRect(const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);

    return {
        .x = camera->position.x - (backbuffer_size.x / (2.0f * camera_scale)),
        .y = camera->position.y - (backbuffer_size.y / (2.0f * camera_scale)),
        .width = backbuffer_size.x / camera_scale,
        .height = backbuffer_size.y / camera_scale,
    };
}

zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    ZCL_ASSERT(backbuffer_size.x > 0 && backbuffer_size.y > 0);

    zcl::t_mat4x4 result = zcl::MatrixCreateIdentity();

    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateScaled({camera_scale, camera_scale}));

    result = zcl::MatrixMultiply(result, zcl::MatrixCreateTranslated((-camera->position * camera_scale) + (zcl::V2IToF(backbuffer_size) / 2.0f)));

    return result;
}
