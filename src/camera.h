#pragma once

struct t_camera;

t_camera *CameraCreate(const zcl::t_v2 position, const zcl::t_f32 lerp_factor, zcl::t_arena *const arena);
void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ);
zcl::t_f32 CameraCalcScale(const zcl::t_v2_i backbuffer_size); // @todo: Remove?
zcl::t_rect_f CameraCalcRect(const t_camera *const camera, const zcl::t_v2_i backbuffer_size);
zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera *const camera, const zcl::t_v2_i backbuffer_size);
