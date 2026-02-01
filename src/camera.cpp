#include "camera.h"

struct t_camera {
    zcl::t_v2 pos;
    zcl::t_f32 scale;
    zcl::t_f32 lerp_factor;
};

t_camera *CameraCreate(const zcl::t_v2 position, const zcl::t_f32 scale, const zcl::t_f32 lerp_factor, zcl::t_arena *const arena) {
    ZCL_ASSERT(lerp_factor >= 0.0f && lerp_factor <= 1.0f);
    ZCL_ASSERT(scale > 0.0f);

    const auto result = zcl::ArenaPush<t_camera>(arena);
    result->pos = position;
    result->scale = scale;
    result->lerp_factor = lerp_factor;

    return result;
}

zcl::t_v2 CameraGetPos(const t_camera *const camera) {
    return camera->pos;
}

zcl::t_f32 CameraGetScale(const t_camera *const camera) {
    return camera->scale;
}

void CameraSetScale(t_camera *const camera, const zcl::t_f32 scale) {
    ZCL_ASSERT(scale > 0.0f);
    camera->scale = scale;
}

void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ) {
    camera->pos = zcl::Lerp(camera->pos, pos_targ, camera->lerp_factor);
}

zcl::t_v2 CameraCalcTopLeft(const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    return camera->pos - (zcl::V2IToF(backbuffer_size) / (2.0f * camera->scale));
}

zcl::t_rect_f CameraCalcRect(const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    return zcl::RectCreateF(CameraCalcTopLeft(camera, backbuffer_size), zcl::V2IToF(backbuffer_size) / camera->scale);
}

zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    ZCL_ASSERT(backbuffer_size.x > 0 && backbuffer_size.y > 0);

    const zcl::t_v2 pos_offs = {
        round(-camera->pos.x * camera->scale),
        round(-camera->pos.y * camera->scale),
    };

    const zcl::t_v2 size_offs = {
        floor(backbuffer_size.x / 2.0f),
        floor(backbuffer_size.y / 2.0f),
    };

    zcl::t_mat4x4 result = zcl::MatrixCreateIdentity();
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateScaled({camera->scale, camera->scale}));
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateTranslated(pos_offs + size_offs));

    return result;
}

zcl::t_v2 BackbufferToCameraPos(const zcl::t_v2 pos, const zcl::t_v2_i backbuffer_size, const t_camera *const camera) {
    return CameraCalcTopLeft(camera, backbuffer_size) + (pos / camera->scale);
}

zcl::t_v2 CameraToBackbufferPos(const zcl::t_v2 pos, const t_camera *const camera, const zcl::t_v2_i backbuffer_size) {
    return (pos - CameraCalcTopLeft(camera, backbuffer_size)) * camera->scale;
}
