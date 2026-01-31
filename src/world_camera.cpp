#include "world_private.h"

zcl::t_f32 CameraCalcScale(const zcl::t_v2_i backbuffer_size) {
    return backbuffer_size.x > 1600 || backbuffer_size.y > 900 ? 1.0f : 1.0f;
}

zcl::t_rect_f CameraCalcRect(const t_camera camera, const zcl::t_v2_i backbuffer_size) {
    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);

    return {
        .x = camera.position.x - (backbuffer_size.x / (2.0f * camera_scale)),
        .y = camera.position.y - (backbuffer_size.y / (2.0f * camera_scale)),
        .width = backbuffer_size.x / camera_scale,
        .height = backbuffer_size.y / camera_scale,
    };
}

zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera camera, const zcl::t_v2_i backbuffer_size) {
    ZCL_ASSERT(backbuffer_size.x > 0 && backbuffer_size.y > 0);

    zcl::t_mat4x4 result = zcl::MatrixCreateIdentity();

    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);
    result = zcl::MatrixMultiply(result, zcl::MatrixCreateScaled({camera_scale, camera_scale}));

    result = zcl::MatrixMultiply(result, zcl::MatrixCreateTranslated((-camera.position * camera_scale) + (zcl::V2IToF(backbuffer_size) / 2.0f)));

    return result;
}

void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ) {
    camera->position = zcl::Lerp(camera->position, pos_targ, k_camera_lerp_factor);
}

zcl::t_rect_i CameraCalcTilemapRect(const t_camera camera, const zcl::t_v2_i backbuffer_size) {
    const zcl::t_f32 camera_scale = CameraCalcScale(backbuffer_size);

    const zcl::t_rect_f camera_rect = CameraCalcRect(camera, backbuffer_size);

    const zcl::t_i32 camera_tilemap_left = static_cast<zcl::t_i32>(floor(zcl::RectGetLeft(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_top = static_cast<zcl::t_i32>(floor(zcl::RectGetTop(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_right = static_cast<zcl::t_i32>(ceil(zcl::RectGetRight(camera_rect) / k_tile_size));
    const zcl::t_i32 camera_tilemap_bottom = static_cast<zcl::t_i32>(ceil(zcl::RectGetBottom(camera_rect) / k_tile_size));

    return zcl::ClampWithinContainer(zcl::RectCreateI(camera_tilemap_left, camera_tilemap_top, camera_tilemap_right - camera_tilemap_left, camera_tilemap_bottom - camera_tilemap_top), zcl::RectCreateI({}, k_tilemap_size));
}
