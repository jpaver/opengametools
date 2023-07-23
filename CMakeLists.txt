project(opengametools CXX)

set(APPS
    apps/vox2fbx.cpp
    apps/vox2obj.cpp
    apps/voxmerge.cpp
    apps/voxseparate.cpp
    demo/demo_vox.cpp
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

foreach (app ${APPS})
    get_filename_component(APP_NAME ${app} NAME_WE) 
    add_executable(${APP_NAME} ${app})
    target_compile_options(${APP_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    )
    install(TARGETS ${APP_NAME} DESTINATION bin)
endforeach()
