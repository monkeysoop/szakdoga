install_subdir('assets', install_dir: meson.current_build_dir())

subdir('core')

main_deps = [
    dependency('sdl2', required: true),
    dependency('SDL2_image', required: true),
    dependency('glew', required: true),
    dependency('opengl', required: true),
    dependency('glu', required: true),
    dependency('imgui', required: true),
    imgui_bindings_dep,
]

main_source_files = [
    'main.cpp',
]

executable(
    meson.project_name(), 
    main_source_files,
    dependencies: main_deps,
    link_with: core_lib,
    include_directories: core_inc,
)