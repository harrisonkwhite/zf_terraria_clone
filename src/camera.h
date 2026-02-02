#pragma once

struct t_camera;

t_camera *CameraCreate(const zcl::t_v2 position, const zcl::t_f32 scale, const zcl::t_f32 lerp_factor, zcl::t_arena *const arena);

zcl::t_v2 CameraGetPos(const t_camera *const camera);

zcl::t_f32 CameraGetScale(const t_camera *const camera);
void CameraSetScale(t_camera *const camera, const zcl::t_f32 scale);

void CameraMove(t_camera *const camera, const zcl::t_v2 pos_targ);

zcl::t_v2 CameraCalcTopLeft(const t_camera *const camera, const zcl::t_v2_i screen_size);

zcl::t_rect_f CameraCalcRect(const t_camera *const camera, const zcl::t_v2_i screen_size);

zcl::t_mat4x4 CameraCalcViewMatrix(const t_camera *const camera, const zcl::t_v2_i screen_size);

zcl::t_v2 CameraToScreenPos(const zcl::t_v2 pos, const t_camera *const camera, const zcl::t_v2_i screen_size);

zcl::t_v2 ScreenToCameraPos(const zcl::t_v2 pos, const zcl::t_v2_i screen_size, const t_camera *const camera);
