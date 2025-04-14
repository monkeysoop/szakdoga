install_subdir('assets', install_dir: meson.current_build_dir())

test_inc = include_directories('include')

test_cases = [
    {'name': 'Texture2D_Test', 'src': 'src/Texture2D_Test.cpp'},
    {'name': 'Skybox_Test', 'src': 'src/Skybox_Test.cpp'},
    {'name': 'Framebuffer_Test', 'src': 'src/Framebuffer_Test.cpp'},
]

test_main = ['gtest_main.cpp']

test_deps = [
    dependency('sdl2', required: true),
    dependency('SDL2_image', required: true),
    dependency('glew', required: true),
    dependency('opengl', required: true),
    dependency('glu', required: true),
    dependency('imgui', required: true),
    dependency('gtest', required: true),
    imgui_bindings_dep,
]

foreach t : test_cases
    exe = executable(
        t['name'],
        sources: test_main + t['src'],
        include_directories: [core_inc, test_inc],
        link_with: core_lib,
        dependencies: test_deps,
    )

    test(t['name'], exe, workdir: meson.current_build_dir())
endforeach