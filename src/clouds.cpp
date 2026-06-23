#if 0
    #include "clouds.h"

    #include "sprites.h"
    #include "camera.h"

struct t_cloud {
    zcl::t_v2 pos;
    zcl::t_i32 sprite_index;
};

struct t_cloud_manager {
    zcl::t_array_mut<t_cloud> clouds;
};

t_cloud_manager *CloudManagerCreate(const zcl::t_i32 cloud_cnt, zcl::t_arena *const arena) {
    const auto result = zcl::ArenaPush<t_cloud_manager>(arena);
    result->clouds = zcl::ArenaPushArray<t_cloud>(arena, cloud_cnt);

    return result;
}

void CloudManagerRenderAll(const t_cloud_manager *const manager, const zgl::t_rendering_context rc, const t_assets *const assets, const t_camera *const camera) {
    for (zcl::t_i32 i = 0; i < manager->clouds.len; i++) {
        const auto camera_view_matrix = CameraCalcViewMatrix(camera, rc.screen_size, 0.05f);
        zgl::RendererPassBegin(rc, rc.screen_size, camera_view_matrix, true, k_sky_color);

        SpriteRender(ek_sprite_id_cloud_0, rc, assets, {20.0f, 20.0f}, zcl::k_origin_center);

        zgl::RendererPassEnd(rc);
    }
}
#endif
