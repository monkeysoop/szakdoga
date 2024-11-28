core_deps = [
    dependency('sdl2', required: true),
    dependency('SDL2_image', required: true),
    dependency('glew', required: true),
    dependency('opengl', required: true),
    dependency('glu', required: true),
    dependency('imgui', required: true),
]

core_source_files = [
    'src/App.cpp',
    'src/Camera.cpp',
    'src/CameraManipulator.cpp',
    'src/CompShader.cpp',
    'src/Texture2D.cpp',
    'src/Skybox.cpp',
    'src/Framebuffer.cpp',
    'src/SDL_GLDebugMessageCallback.cpp',
]

core_inc = include_directories('include')

core_lib = static_library(
    'core',
    sources: core_source_files,
    include_directories: core_inc,
    dependencies: core_deps,
)