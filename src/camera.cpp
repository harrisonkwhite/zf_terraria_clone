#include "camera.h"

struct t_camera {
    zcl::t_v2 pos;
    zcl::t_f32 scale;
    zcl::t_f32 lerp_factor;
};

t_camera *CameraCreate(const zcl::t_f32 scale, const zcl::t_f32 lerp_factor, zcl::t_arena *const arena) {
    ZCL_ASSERT(lerp_factor >= 0.0f && lerp_factor <= 1.0f);
    ZCL_ASSERT(scale > 0.0f);

    const auto result = zcl::ArenaPush<t_camera>(arena);
    result->scale = scale;
    result->lerp_factor = lerp_factor;

    return result;
}

zcl::t_v2 CameraGetPosition(const t_camera *const camera) {
    return camera->pos;
}

void CameraSetPosition(t_camera *const camera, const zcl::t_v2 pos) {
    camera->pos = pos;
}

zcl::t_f32 CameraGetScale(const t_camera *const camera) {
    return camera->scale;
}

void CameraSetScale(t_camera *const camera, const zcl::t_f32 scale) {
    ZCL_ASSERT(scale > 0.0f);
    camera->scale = scale;
}

zcl::t_v2 CameraGetSize(const t_camera *const camera, const zcl::t_v2_i screen_size) {
    return zcl::V2IToF(screen_size) / camera->scale;
}

void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ) {
    camera->pos = zcl::Lerp(camera->pos, pos_targ, camera->lerp_factor);
}

void CameraClamp(t_camera *const camera, const zcl::t_rect_f container, const zcl::t_v2_i screen_size) {
    const auto camera_size = CameraGetSize(camera, screen_size);

    camera->pos.x = zcl::Clamp(camera->pos.x, zcl::RectGetLeft(container), zcl::RectGetRight(container) - camera_size.x);
    camera->pos.y = zcl::Clamp(camera->pos.y, zcl::RectGetTop(container), zcl::RectGetBottom(container) - camera_size.y);
}

zcl::t_rect_f CameraCalcRect(const t_camera *const camera, const zcl::t_v2_i screen_size) {
    return zcl::RectCreateF(camera->pos, CameraGetSize(camera, screen_size));
}

zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera *const camera, const zcl::t_v2_i screen_size, const zcl::t_f32 parallax) {
    ZCL_ASSERT(screen_size.x > 0 && screen_size.y > 0);
    ZCL_ASSERT(parallax >= 0.0f && parallax <= 1.0f);

    const zcl::t_v2 pos_offs = {
        zcl::Round(-camera->pos.x * camera->scale * parallax),
        zcl::Round(-camera->pos.y * camera->scale * parallax),
    };

    const zcl::t_v2 size_offs = {
        zcl::Floor(screen_size.x / 2.0f),
        zcl::Floor(screen_size.y / 2.0f),
    };

    zcl::t_mat4x4 result = zcl::MatrixCreateIdentity();
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateScaled({camera->scale, camera->scale}));
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateTranslated(pos_offs));

    return result;
}

zcl::t_v2 CameraToScreenPos(const zcl::t_v2 pos, const t_camera *const camera, const zcl::t_v2_i screen_size) {
    return (pos - CameraGetPosition(camera)) * camera->scale;
}

zcl::t_v2 ScreenToCameraPos(const zcl::t_v2 pos, const zcl::t_v2_i screen_size, const t_camera *const camera) {
    return CameraGetPosition(camera) + (pos / camera->scale);
}
