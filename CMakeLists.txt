project(opengametools CXX)

set(APPS
    vox2fbx
    vox2obj
    voxmerge
    voxseparate
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

foreach (app ${APPS})
    add_executable(${app} apps/${app}.cpp)
    target_compile_options(${app} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    )
    install(TARGETS ${app} DESTINATION bin)
endforeach()
