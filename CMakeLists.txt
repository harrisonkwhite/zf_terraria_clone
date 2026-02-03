cmake_minimum_required(VERSION 3.28)

project(zf_terraria_clone LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(zeta_framework)

add_executable(${PROJECT_NAME}
    src/entry.cpp
    src/game.cpp
    src/assets.cpp
    src/ui_helpers.cpp
    src/title_screen.cpp
    src/tiles.cpp
    src/world.cpp
    src/world_items.cpp
    src/world_tilemap.cpp
    src/world_inventories.cpp
    src/camera.cpp

    src/pch.h
    src/game.h
    src/assets.h
    src/sprites.h
    src/ui_helpers.h
    src/title_screen.h
    src/camera.h
    src/items.h
    src/npcs.h
    src/world_public.h
    src/world_private.h
)

target_precompile_headers(${PROJECT_NAME} PRIVATE src/pch.h)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE zf_game_lib)
target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
        $<$<NOT:$<CONFIG:Debug>>:/ENTRY:mainCRTStartup>
    )
endif()

add_custom_target(build_assets
    COMMAND zf_asset_builder "zf_asset_building_instrs.json"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "assets" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Building assets..."
    VERBATIM
)

add_dependencies(${PROJECT_NAME} build_assets)
