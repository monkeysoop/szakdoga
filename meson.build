project(
    'main',
    'cpp',
    default_options: ['cpp_std=c++17'],
    version: '0.1.0',
)

imgui_bindings_dep = subproject('imgui_bindings', required: true).get_variable('imgui_bindings_dep')

subdir('app')