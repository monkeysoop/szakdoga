project(
    'imgui_bindings',
    'cpp', 
    default_options: ['cpp_std=c++17'], 
    version: '0.1.0',
)

project_source = [
    'src/imgui_impl_sdl2.cpp',
    'src/imgui_impl_opengl3.cpp',
]

project_inc = include_directories('include')

project_deps = [
    dependency('sdl2', required: true),
    dependency('opengl', required: true),
    dependency('imgui', required: true),
]

# has to be static otherwise segfault (null pointer dereference when creating sdl2 gl context for imgui in main)
project_lib = static_library(
    meson.project_name(),
    project_source,
    dependencies: project_deps,
    include_directories: project_inc,
)

set_variable(
    '' + meson.project_name() + '_dep',
    declare_dependency(
        link_with: project_lib,
        include_directories: project_inc,
    ),
)
