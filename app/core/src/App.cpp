#include "App.hpp"
#include "SDL_GLDebugMessageCallback.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glew.h>

#include <iostream>
#include <cmath>
#include <cassert>
#include <fstream>
#include <chrono>



namespace szakdoga::core {
    const unsigned LOCAL_WORKGROUP_SIZE_X = 8;
    const unsigned LOCAL_WORKGROUP_SIZE_Y = 8;

    App::App(unsigned width, unsigned height) : 
        m_width{width}, 
        m_height{height}, 
        m_camera{}, 
        m_camera_manipulator{}, 
        m_framebuffer{width, height},
        m_sdf_scene{SDFSceneType::NEWTONS_CRADLE},
        m_render_mode{RenderModeType::NORMAL},
        m_tracing_type{TracingType::NAIVE},
        m_sphere_tracing_type{SphereTracingType::NAIVE},
        m_cone_trace_sphere_tracing_type{SphereTracingType::NAIVE},
        m_cone_trace_final_sphere_tracing_type{SphereTracingType::NAIVE},
        m_sphere_trace_shader{
            std::filesystem::path{"assets"} / "sphere_tracer.comp", 
            {
                std::filesystem::path{"assets"} / "ray.include",
                std::filesystem::path{"assets"} / "value.include", 
                std::filesystem::path{"assets"} / "sdf.include",
                std::filesystem::path{"assets"} / "sdf_newtons_cradle.include", 
                std::filesystem::path{"assets"} / "sdf_car.include", 
                std::filesystem::path{"assets"} / "sdf_temple.include", 
                std::filesystem::path{"assets"} / "tracing_result.include",
                std::filesystem::path{"assets"} / "tracers.include", 
                std::filesystem::path{"assets"} / "renderer.include"
            }, 
            {
                {"SDF_SCENE", std::to_string(static_cast<unsigned>(m_sdf_scene))},
                {"RENDER_MODE", std::to_string(static_cast<unsigned>(m_render_mode))}, 
                {"SPHERE_TRACING_TYPE", std::to_string(static_cast<unsigned>(m_sphere_tracing_type))}
            }
        },
        m_cone_trace_shader{
            std::filesystem::path{"assets"} / "cone_tracer.comp", 
            {
                std::filesystem::path{"assets"} / "ray.include",
                std::filesystem::path{"assets"} / "value.include", 
                std::filesystem::path{"assets"} / "sdf.include",
                std::filesystem::path{"assets"} / "sdf_newtons_cradle.include", 
                std::filesystem::path{"assets"} / "sdf_car.include", 
                std::filesystem::path{"assets"} / "sdf_temple.include"
            },
            {
                {"SDF_SCENE", std::to_string(static_cast<unsigned>(m_sdf_scene))},
                {"SPHERE_TRACING_TYPE", std::to_string(static_cast<unsigned>(m_cone_trace_sphere_tracing_type))}
            }
        },
        m_cone_trace_final_shader{
            std::filesystem::path{"assets"} / "cone_tracer_final_stage.comp", 
            {
                std::filesystem::path{"assets"} / "ray.include",
                std::filesystem::path{"assets"} / "value.include", 
                std::filesystem::path{"assets"} / "sdf.include",
                std::filesystem::path{"assets"} / "sdf_newtons_cradle.include", 
                std::filesystem::path{"assets"} / "sdf_car.include", 
                std::filesystem::path{"assets"} / "sdf_temple.include", 
                std::filesystem::path{"assets"} / "tracing_result.include",
                std::filesystem::path{"assets"} / "tracers.include", 
                std::filesystem::path{"assets"} / "renderer.include"
            },
            {
                {"SDF_SCENE", std::to_string(static_cast<unsigned>(m_sdf_scene))},
                {"RENDER_MODE", std::to_string(static_cast<unsigned>(m_render_mode))},
                {"SPHERE_TRACING_TYPE", std::to_string(static_cast<unsigned>(m_cone_trace_final_sphere_tracing_type))}
            }
        },
        m_cone_trace_precompute_shader{std::filesystem::path{"assets"} / "cone_precompute.comp"},
        m_cone_trace_distance_iteration_texture_1{width, height, GL_RG32F},
        m_cone_trace_distance_iteration_texture_2{width, height, GL_RG32F},
        m_initial_cone_size{4},
        m_cone_trace_precomputed_texture{
            static_cast<unsigned>((width + m_initial_cone_size - 1) / 2), 
            static_cast<unsigned>((height + m_initial_cone_size - 1) / 2), 
            GL_RGBA32F, 
            static_cast<unsigned>(std::log2(m_initial_cone_size))
        },
        m_skybox{},
        m_time_in_seconds{0.0f},
        m_epsilon{0.001f},
        m_max_distance{1000.0f},
        m_max_iteration_count{100},
        m_relaxed_step_multiplier{1.6f},
        m_enhanced_step_multiplier{0.9f},
        m_enhanced_max_step_factor{10.0f},
        m_cone_trace_intermediate_epsilon{0.02f},
        m_cone_trace_relaxed_step_multiplier{1.6f},
        m_cone_trace_enhanced_step_multiplier{0.9f},
        m_cone_trace_enhanced_max_step_factor{10.0f},
        m_cone_trace_final_relaxed_step_multiplier{1.6f},
        m_cone_trace_final_enhanced_step_multiplier{0.9f},
        m_cone_trace_final_enhanced_max_step_factor{10.0f},
        m_benchmark_baseline_iteration_count{1000},
        m_benchmark_min_iteration_count{10},
        m_benchmark_max_iteration_count{100},
        m_benchmark_iteration_count_spacing{10},
        m_benchmark_performance_iteration_count{100},
        m_benchmark_performance_number_of_runs{10}
    {
        GLint context_flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &context_flags);
        if (context_flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
            glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DONT_CARE, 0, nullptr, GL_FALSE);
            glDebugMessageCallback(SDL_GLDebugMessageCallback, nullptr);
        }

        m_camera.SetView(glm::vec3(2, 2, 2),   // From where we look at the scene - eye
                         glm::vec3(0, 0, 0),   // Which point of the scene we are looking at - at
                         glm::vec3(0, 1, 0)    // Upwards direction - up
        );

        m_camera_manipulator.SetCamera(&m_camera);

        PrecomputeCones();
    }

    App::~App() {}

    void App::Update(float elapsed_time_in_seconds, float delta_time_in_seconds) {
        m_time_in_seconds = elapsed_time_in_seconds;
        m_camera_manipulator.Update(delta_time_in_seconds);
    }

    void App::Render() {
        if (m_tracing_type == TracingType::CONE) {
            ConeTraceRender();
        } else {
            SphereTraceRender();
        }
    }

    void App::RenderImGui() {
        if (ImGui::Begin("Settings")) {
            if (ImGui::CollapsingHeader("sdf scene")) {
                SDFSceneType old_sdf_scene = m_sdf_scene;
                int sdf_scene = static_cast<int>(m_sdf_scene);
                ImGui::RadioButton("newton cradle", &sdf_scene, static_cast<int>(SDFSceneType::NEWTONS_CRADLE));
                ImGui::SameLine();
                ImGui::RadioButton("car", &sdf_scene, static_cast<int>(SDFSceneType::CAR));
                ImGui::SameLine();
                ImGui::RadioButton("temple", &sdf_scene, static_cast<int>(SDFSceneType::TEMPLE));
                m_sdf_scene = static_cast<SDFSceneType>(sdf_scene);

                if (m_sdf_scene != old_sdf_scene) {
                    RecompileSphereTracer(m_sdf_scene, m_render_mode, m_sphere_tracing_type);
                    RecompileConeTracer(m_sdf_scene, m_cone_trace_sphere_tracing_type);
                    RecompileConeFinalTracer(m_sdf_scene, m_render_mode, m_cone_trace_final_sphere_tracing_type);
                }
            }

            if (ImGui::CollapsingHeader("render mode")) {
                RenderModeType old_render_mode = m_render_mode;
                int mode = static_cast<int>(m_render_mode);
                ImGui::RadioButton("normal", &mode, static_cast<int>(RenderModeType::NORMAL));
                ImGui::SameLine();
                ImGui::RadioButton("iteration count", &mode, static_cast<int>(RenderModeType::ITERATION_COUNT));
                ImGui::SameLine();
                ImGui::RadioButton("depth", &mode, static_cast<int>(RenderModeType::DEPTH));
                m_render_mode = static_cast<RenderModeType>(mode);

                if (m_render_mode != old_render_mode) {
                    RecompileSphereTracer(m_sdf_scene, m_render_mode, m_sphere_tracing_type);
                    RecompileConeFinalTracer(m_sdf_scene, m_render_mode, m_cone_trace_final_sphere_tracing_type);
                }
            }

            if (ImGui::CollapsingHeader("general settings")) {
                ImGui::SliderFloat("epsilon", &m_epsilon, 0.000001f, 0.1f, "%.6f", ImGuiSliderFlags_Logarithmic);
                ImGui::SliderFloat("max distance", &m_max_distance, 1.0f, 10000.0f, "%.0f", ImGuiSliderFlags_Logarithmic);
                int iter_count = static_cast<int>(m_max_iteration_count);
                ImGui::SliderInt("max iteration count", &iter_count, 10, 1000, "%d", ImGuiSliderFlags_Logarithmic);
                m_max_iteration_count = static_cast<unsigned>(iter_count);
            }

            if (ImGui::CollapsingHeader("sphere tracing type")) {
                int type = static_cast<int>(m_tracing_type);
                ImGui::RadioButton("naive", &type, static_cast<int>(TracingType::NAIVE));
                ImGui::SameLine();
                ImGui::RadioButton("relaxed", &type, static_cast<int>(TracingType::RELAXED));
                ImGui::SameLine();
                ImGui::RadioButton("enhanced", &type, static_cast<int>(TracingType::ENHANCED));
                ImGui::SameLine();
                ImGui::RadioButton("cone", &type, static_cast<int>(TracingType::CONE));
                m_tracing_type = static_cast<TracingType>(type);
                if (m_tracing_type != TracingType::CONE) {
                    SphereTracingType old_sphere_tracing_type = m_sphere_tracing_type;
                    m_sphere_tracing_type = static_cast<SphereTracingType>(m_tracing_type);
                    if (old_sphere_tracing_type != m_sphere_tracing_type) {
                        RecompileSphereTracer(m_sdf_scene, m_render_mode, m_sphere_tracing_type);
                    }
                }

                if (ImGui::CollapsingHeader("relaxed specific settings")) {
                    ImGui::SliderFloat("relaxed step size multiplier", &m_relaxed_step_multiplier, 1.0f, 2.0f);
                }

                if (ImGui::CollapsingHeader("enhanced specific settings")) {
                    ImGui::SliderFloat("enhanced step size multiplier", &m_enhanced_step_multiplier, 0.7f, 1.0f);
                    ImGui::SliderFloat("enhanced max step factor", &m_enhanced_max_step_factor, 1.0f, 20.0f);
                }

                if (ImGui::CollapsingHeader("cone specific settings")) {
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("change cone size");
                    ImGui::SameLine();
                    ImGui::PushItemFlag(ImGuiItemFlags_ButtonRepeat, true);
                    if (ImGui::ArrowButton("decrease cone size", ImGuiDir_Left) && m_initial_cone_size > 1) {
                        m_initial_cone_size /= 2;
                        m_cone_trace_precomputed_texture.Resize(
                            static_cast<unsigned>((m_width + m_initial_cone_size - 1) / 2), 
                            static_cast<unsigned>((m_height + m_initial_cone_size - 1) / 2), 
                            static_cast<unsigned>(std::log2(m_initial_cone_size))
                        );
                        PrecomputeCones();
                    }
                    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                    if (ImGui::ArrowButton("increase cone size", ImGuiDir_Right) && m_initial_cone_size < 256) {
                        m_initial_cone_size *= 2;
                        m_cone_trace_precomputed_texture.Resize(
                            static_cast<unsigned>((m_width + m_initial_cone_size - 1) / 2), 
                            static_cast<unsigned>((m_height + m_initial_cone_size - 1) / 2), 
                            static_cast<unsigned>(std::log2(m_initial_cone_size))
                        );
                        PrecomputeCones();
                    }
                    ImGui::PopItemFlag();
                    ImGui::SameLine();
                    ImGui::Text("%d", m_initial_cone_size);

                    ImGui::SliderFloat("cone intermediate epsilon", &m_cone_trace_intermediate_epsilon, 0.00001f, 1.0f, "%.6f", ImGuiSliderFlags_Logarithmic);

                    SphereTracingType old_cone_trace_sphere_tracing_type = m_cone_trace_sphere_tracing_type;
                    int cone_trace_type = static_cast<int>(m_cone_trace_sphere_tracing_type);
                    ImGui::RadioButton("cone trace naive", &cone_trace_type, static_cast<int>(SphereTracingType::NAIVE));
                    ImGui::RadioButton("cone trace relaxed", &cone_trace_type, static_cast<int>(SphereTracingType::RELAXED));
                    ImGui::RadioButton("cone trace enhanced", &cone_trace_type, static_cast<int>(SphereTracingType::ENHANCED));
                    m_cone_trace_sphere_tracing_type = static_cast<SphereTracingType>(cone_trace_type);
                    if (m_cone_trace_sphere_tracing_type != old_cone_trace_sphere_tracing_type) {
                        RecompileConeTracer(m_sdf_scene, m_cone_trace_sphere_tracing_type);
                    }

                    if (ImGui::CollapsingHeader("cone relaxed specific settings")) {
                        ImGui::SliderFloat("cone relaxed step size multiplier", &m_cone_trace_relaxed_step_multiplier, 1.0f, 2.0f);
                    }

                    if (ImGui::CollapsingHeader("cone enhanced specific settings")) {
                        ImGui::SliderFloat("cone enhanced step size multiplier", &m_cone_trace_enhanced_step_multiplier, 0.7f, 1.0f);
                        ImGui::SliderFloat("cone enhanced max step factor", &m_cone_trace_enhanced_max_step_factor, 1.0f, 20.0f);
                    }

                    SphereTracingType old_cone_trace_final_sphere_tracing_type = m_cone_trace_final_sphere_tracing_type;
                    int cone_trace_final_type = static_cast<int>(m_cone_trace_final_sphere_tracing_type);
                    ImGui::RadioButton("cone trace final naive", &cone_trace_final_type, static_cast<int>(SphereTracingType::NAIVE));
                    ImGui::RadioButton("cone trace final relaxed", &cone_trace_final_type, static_cast<int>(SphereTracingType::RELAXED));
                    ImGui::RadioButton("cone trace final enhanced", &cone_trace_final_type, static_cast<int>(SphereTracingType::ENHANCED));
                    m_cone_trace_final_sphere_tracing_type = static_cast<SphereTracingType>(cone_trace_final_type);
                    if (m_cone_trace_final_sphere_tracing_type != old_cone_trace_final_sphere_tracing_type) {
                        RecompileConeFinalTracer(m_sdf_scene, m_render_mode, m_cone_trace_final_sphere_tracing_type);
                    }

                    if (ImGui::CollapsingHeader("cone final relaxed specific settings")) {
                        ImGui::SliderFloat("cone final relaxed step size multiplier", &m_cone_trace_final_relaxed_step_multiplier, 1.0f, 2.0f);
                    }

                    if (ImGui::CollapsingHeader("cone final enhanced specific settings")) {
                        ImGui::SliderFloat("cone final enhanced step size multiplier", &m_cone_trace_final_enhanced_step_multiplier, 0.7f, 1.0f);
                        ImGui::SliderFloat("cone final enhanced max step factor", &m_cone_trace_final_enhanced_max_step_factor, 1.0f, 20.0f);
                    }
                }
            }

            if (ImGui::CollapsingHeader("benchmark settings")) {
                int benchmark_baseline_iteration_count = static_cast<int>(m_benchmark_baseline_iteration_count);
                int benchmark_min_iteration_count = static_cast<int>(m_benchmark_min_iteration_count);
                int benchmark_max_iteration_count = static_cast<int>(m_benchmark_max_iteration_count);
                int benchmark_iteration_count_spacing = static_cast<int>(m_benchmark_iteration_count_spacing);
                int benchmark_performance_iteration_count = static_cast<int>(m_benchmark_performance_iteration_count);
                int benchmark_performance_number_of_runs = static_cast<int>(m_benchmark_performance_number_of_runs);
                ImGui::SliderInt("baseline iteration count", &benchmark_baseline_iteration_count, 10, 1000, "%d", ImGuiSliderFlags_Logarithmic);
                if (ImGui::SliderInt("min iteration count", &benchmark_min_iteration_count, 10, 1000, "%d", ImGuiSliderFlags_Logarithmic)) {
                    if (benchmark_min_iteration_count > benchmark_max_iteration_count) {
                        benchmark_max_iteration_count = benchmark_min_iteration_count;
                    }
                }
                if (ImGui::SliderInt("max iteration count", &benchmark_max_iteration_count, 10, 1000, "%d", ImGuiSliderFlags_Logarithmic)) {
                    if (benchmark_max_iteration_count < benchmark_min_iteration_count) {
                        benchmark_min_iteration_count = benchmark_max_iteration_count;
                    }
                }
                ImGui::SliderInt("iteration count spacing", &benchmark_iteration_count_spacing, 1, 1000, "%d", ImGuiSliderFlags_Logarithmic);
                ImGui::SliderInt("performance iteration count", &benchmark_performance_iteration_count, 10, 1000, "%d", ImGuiSliderFlags_Logarithmic);
                ImGui::SliderInt("performance number of runs", &benchmark_performance_number_of_runs, 0, 100);
                m_benchmark_baseline_iteration_count = static_cast<unsigned>(benchmark_baseline_iteration_count);
                m_benchmark_min_iteration_count = static_cast<unsigned>(benchmark_min_iteration_count);
                m_benchmark_max_iteration_count = static_cast<unsigned>(benchmark_max_iteration_count);
                m_benchmark_iteration_count_spacing = static_cast<unsigned>(benchmark_iteration_count_spacing);
                m_benchmark_performance_iteration_count = static_cast<unsigned>(benchmark_performance_iteration_count);
                m_benchmark_performance_number_of_runs = static_cast<unsigned>(benchmark_performance_number_of_runs);
            }

            if (ImGui::Button("Benchmark")) {
                Benchmark();
            }
        }
        ImGui::End();
    }

    void App::KeyboardDown(const SDL_KeyboardEvent& key) {
        m_camera_manipulator.KeyboardDown(key);
    }

    void App::KeyboardUp(const SDL_KeyboardEvent& key) {
        m_camera_manipulator.KeyboardUp(key);
    }

    void App::MouseMove(const SDL_MouseMotionEvent& mouse) {
        m_camera_manipulator.MouseMove(mouse);
    }

    void App::MouseWheel(const SDL_MouseWheelEvent& wheel) {
        m_camera_manipulator.MouseWheel(wheel);
        PrecomputeCones();
    }

    void App::Resize(unsigned width, unsigned height) {
        m_width = width;
        m_height = height;

        glViewport(0, 0, width, height);
        m_framebuffer.Resize(width, height);

        m_camera.SetAspect(static_cast<float>(width) / static_cast<float>(height));

        m_cone_trace_distance_iteration_texture_1.Resize(width, height);
        m_cone_trace_distance_iteration_texture_2.Resize(width, height);

        m_cone_trace_precomputed_texture.Resize(
            static_cast<unsigned>((width + m_initial_cone_size - 1) / 2), 
            static_cast<unsigned>((height + m_initial_cone_size - 1) / 2)
        );
        PrecomputeCones();
    }

    void App::PrecomputeCones() {
        m_cone_trace_precompute_shader.Use();

        unsigned cone_size = 2;
        unsigned level = 0;

        while (cone_size <= m_initial_cone_size) {
            glUniformMatrix4fv(m_cone_trace_precompute_shader.ul("u_inv_proj_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetProj())));
            glUniform1ui(m_cone_trace_precompute_shader.ul("u_width"), static_cast<GLuint>(m_width));
            glUniform1ui(m_cone_trace_precompute_shader.ul("u_height"), static_cast<GLuint>(m_height));

            glUniform1ui(m_cone_trace_precompute_shader.ul("u_cone_size"), static_cast<GLuint>(cone_size));

            m_cone_trace_precomputed_texture.Bind(0, GL_WRITE_ONLY, level);

            m_cone_trace_precompute_shader.Dispatch(
                DivideAndRoundUp(DivideAndRoundUp(m_width, cone_size), LOCAL_WORKGROUP_SIZE_X),
                DivideAndRoundUp(DivideAndRoundUp(m_height, cone_size), LOCAL_WORKGROUP_SIZE_Y),
                1
            );
            m_cone_trace_precompute_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            cone_size *= 2;
            level++;
        }
    }

    void App::SphereTraceRender() {
        m_framebuffer.Bind();

        m_sphere_trace_shader.Use();

        glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.GetTextureID());
        glUniform1i(m_sphere_trace_shader.ul("u_skyboxTexture"), 0);

        glUniformMatrix4fv(m_sphere_trace_shader.ul("u_inv_view_proj_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewProj())));
        glUniform3fv(m_sphere_trace_shader.ul("u_position"), 1, glm::value_ptr(m_camera.GetEye()));
        glUniform1ui(m_sphere_trace_shader.ul("u_width"), static_cast<GLuint>(m_width));
        glUniform1ui(m_sphere_trace_shader.ul("u_height"), static_cast<GLuint>(m_height));

        glUniform1f(m_sphere_trace_shader.ul("u_time_in_seconds"), static_cast<GLfloat>(m_time_in_seconds));
        glUniform1f(m_sphere_trace_shader.ul("u_epsilon"), static_cast<GLfloat>(m_epsilon));
        glUniform1f(m_sphere_trace_shader.ul("u_max_distance"), static_cast<GLfloat>(m_max_distance));
        glUniform1f(m_sphere_trace_shader.ul("u_max_iteration_count"), static_cast<GLfloat>(m_max_iteration_count));

        glUniform1f(m_sphere_trace_shader.ul("u_relaxed_step_multiplier"), static_cast<GLfloat>(m_relaxed_step_multiplier));
        glUniform1f(m_sphere_trace_shader.ul("u_enhanced_step_multiplier"), static_cast<GLfloat>(m_enhanced_step_multiplier));
        glUniform1f(m_sphere_trace_shader.ul("u_enhanced_max_step_factor"), static_cast<GLfloat>(m_enhanced_max_step_factor));

        m_sphere_trace_shader.Dispatch(
            DivideAndRoundUp(m_width, LOCAL_WORKGROUP_SIZE_X),
            DivideAndRoundUp(m_height, LOCAL_WORKGROUP_SIZE_Y),
            1
        );

        m_sphere_trace_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        m_framebuffer.UnBind();
        m_framebuffer.Blit();
    }

    void App::ConeTraceRender() {
        m_cone_trace_shader.Use();

        unsigned cone_size = m_initial_cone_size;
        bool t = true;
        unsigned level = static_cast<unsigned>(std::log2(m_initial_cone_size)) - 1;

        GLboolean first_pass = GL_TRUE;

        while (cone_size > 1) {
            if (t) {
                m_cone_trace_distance_iteration_texture_1.Bind(0, GL_READ_ONLY);
                m_cone_trace_distance_iteration_texture_2.Bind(1, GL_WRITE_ONLY);
            } else {
                m_cone_trace_distance_iteration_texture_2.Bind(0, GL_READ_ONLY);
                m_cone_trace_distance_iteration_texture_1.Bind(1, GL_WRITE_ONLY);
            }
            m_cone_trace_precomputed_texture.Bind(2, GL_READ_ONLY, level);

            glUniformMatrix4fv(m_cone_trace_shader.ul("u_inv_view_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewMatrix())));
            glUniform3fv(m_cone_trace_shader.ul("u_position"), 1, glm::value_ptr(m_camera.GetEye()));
            glUniform1ui(m_cone_trace_shader.ul("u_width"), static_cast<GLfloat>(m_width));
            glUniform1ui(m_cone_trace_shader.ul("u_height"), static_cast<GLfloat>(m_height));

            glUniform1f(m_cone_trace_shader.ul("u_time_in_seconds"), static_cast<GLfloat>(m_time_in_seconds));
            glUniform1f(m_cone_trace_shader.ul("u_epsilon"), static_cast<GLfloat>(m_cone_trace_intermediate_epsilon));
            glUniform1f(m_cone_trace_shader.ul("u_max_distance"), static_cast<GLfloat>(m_max_distance));
            glUniform1f(m_cone_trace_shader.ul("u_max_iteration_count"), static_cast<GLfloat>(m_max_iteration_count));

            glUniform1i(m_cone_trace_shader.ul("u_first_pass"), static_cast<GLint>(first_pass));

            glUniform1f(m_cone_trace_shader.ul("u_relaxed_step_multiplier"), static_cast<GLfloat>(m_cone_trace_relaxed_step_multiplier));
            glUniform1f(m_cone_trace_shader.ul("u_enhanced_step_multiplier"), static_cast<GLfloat>(m_cone_trace_enhanced_step_multiplier));
            glUniform1f(m_cone_trace_shader.ul("u_enhanced_max_step_factor"), static_cast<GLfloat>(m_cone_trace_enhanced_max_step_factor));

            m_cone_trace_shader.Dispatch(
                DivideAndRoundUp(DivideAndRoundUp(m_width, cone_size), LOCAL_WORKGROUP_SIZE_X),
                DivideAndRoundUp(DivideAndRoundUp(m_height, cone_size), LOCAL_WORKGROUP_SIZE_Y),
                1
            );

            m_cone_trace_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            cone_size /= 2;
            t = !t;
            level--;

            first_pass = GL_FALSE;
        }

        m_framebuffer.Bind();

        m_cone_trace_final_shader.Use();

        if (t) {
            m_cone_trace_distance_iteration_texture_1.Bind(1, GL_READ_ONLY);
        } else {
            m_cone_trace_distance_iteration_texture_2.Bind(1, GL_READ_ONLY);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.GetTextureID());
        glUniform1i(m_cone_trace_final_shader.ul("u_skyboxTexture"), 0);

        glUniformMatrix4fv(m_cone_trace_final_shader.ul("u_inv_view_proj_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewProj())));
        glUniform3fv(m_cone_trace_final_shader.ul("u_position"), 1, glm::value_ptr(m_camera.GetEye()));
        glUniform1ui(m_cone_trace_final_shader.ul("u_width"), static_cast<GLuint>(m_width));
        glUniform1ui(m_cone_trace_final_shader.ul("u_height"), static_cast<GLuint>(m_height));

        glUniform1f(m_cone_trace_final_shader.ul("u_time_in_seconds"), static_cast<GLfloat>(m_time_in_seconds));
        glUniform1f(m_cone_trace_final_shader.ul("u_epsilon"), static_cast<GLfloat>(m_epsilon));
        glUniform1f(m_cone_trace_final_shader.ul("u_max_distance"), static_cast<GLfloat>(m_max_distance));
        glUniform1f(m_cone_trace_final_shader.ul("u_max_iteration_count"), static_cast<GLfloat>(m_max_iteration_count));

        glUniform1i(m_cone_trace_final_shader.ul("u_first_pass"), static_cast<GLint>(first_pass));

        glUniform1f(m_cone_trace_final_shader.ul("u_relaxed_step_multiplier"), static_cast<GLfloat>(m_cone_trace_final_relaxed_step_multiplier));
        glUniform1f(m_cone_trace_final_shader.ul("u_enhanced_step_multiplier"), static_cast<GLfloat>(m_cone_trace_final_enhanced_step_multiplier));
        glUniform1f(m_cone_trace_final_shader.ul("u_enhanced_max_step_factor"), static_cast<GLfloat>(m_cone_trace_final_enhanced_max_step_factor));

        m_cone_trace_final_shader.Dispatch(
            DivideAndRoundUp(m_width, LOCAL_WORKGROUP_SIZE_X),
            DivideAndRoundUp(m_height, LOCAL_WORKGROUP_SIZE_Y),
            1
        );

        m_cone_trace_final_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
        m_framebuffer.UnBind();
        m_framebuffer.Blit();
    }

    unsigned App::DivideAndRoundUp(unsigned number, unsigned divisor) {
        return (number + divisor - 1) / divisor;
    }

    void App::Benchmark() {
        std::filesystem::path visual_path{std::filesystem::path{"benchmarks"} / "visual"};
        std::filesystem::path performance_path{std::filesystem::path{"benchmarks"} / "performance"};

        float old_time = m_time_in_seconds;
        unsigned old_iter_count = m_max_iteration_count;

        m_time_in_seconds = 0.0;

        std::filesystem::create_directories(visual_path);
        std::filesystem::create_directory(performance_path);

        RecompileSphereTracer(m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::NAIVE);
        m_max_iteration_count = m_benchmark_baseline_iteration_count;
        SphereTraceRender();
        m_framebuffer.Screenshot(visual_path / "screenshot_baseline.png");

        BenchmarkSingleSphere(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::NAIVE);
        BenchmarkSingleSphere(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::RELAXED);
        BenchmarkSingleSphere(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::ENHANCED);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::NAIVE,    SphereTracingType::NAIVE);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::NAIVE,    SphereTracingType::RELAXED);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::NAIVE,    SphereTracingType::ENHANCED);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::RELAXED,  SphereTracingType::NAIVE);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::RELAXED,  SphereTracingType::RELAXED);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::RELAXED,  SphereTracingType::ENHANCED);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::ENHANCED, SphereTracingType::NAIVE);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::ENHANCED, SphereTracingType::RELAXED);
        BenchmarkSingleCone(visual_path, m_sdf_scene, RenderModeType::NORMAL, SphereTracingType::ENHANCED, SphereTracingType::ENHANCED);

        BenchmarkSingleSphere(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::NAIVE);
        BenchmarkSingleSphere(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::RELAXED);
        BenchmarkSingleSphere(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::ENHANCED);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::NAIVE,    SphereTracingType::NAIVE);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::NAIVE,    SphereTracingType::RELAXED);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::NAIVE,    SphereTracingType::ENHANCED);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::RELAXED,  SphereTracingType::NAIVE);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::RELAXED,  SphereTracingType::RELAXED);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::RELAXED,  SphereTracingType::ENHANCED);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::ENHANCED, SphereTracingType::NAIVE);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::ENHANCED, SphereTracingType::RELAXED);
        BenchmarkSingleCone(performance_path, m_sdf_scene, RenderModeType::ITERATION_COUNT, SphereTracingType::ENHANCED, SphereTracingType::ENHANCED);

        m_time_in_seconds = old_time;
        m_max_iteration_count = old_iter_count;

        RecompileSphereTracer(m_sdf_scene, m_render_mode, m_sphere_tracing_type);
        RecompileConeTracer(m_sdf_scene, m_cone_trace_sphere_tracing_type);
        RecompileConeFinalTracer(m_sdf_scene, m_render_mode, m_cone_trace_final_sphere_tracing_type);
    }

    void App::BenchmarkSingleSphere(const std::filesystem::path& base_path, SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType sphere_tracing_type) {
        RecompileSphereTracer(sdf_scene, render_mode, sphere_tracing_type);

        std::filesystem::path path{};
        switch (sphere_tracing_type) {
            case SphereTracingType::NAIVE:    path = base_path / "naive";    break;
            case SphereTracingType::RELAXED:  path = base_path / "relaxed";  break;
            case SphereTracingType::ENHANCED: path = base_path / "enhanced"; break;
            default: break;
        }

        std::filesystem::create_directory(path);

        for (m_max_iteration_count = m_benchmark_min_iteration_count; m_max_iteration_count <= m_benchmark_max_iteration_count; m_max_iteration_count += m_benchmark_iteration_count_spacing) {
            std::string filename{"screenshot_" + std::to_string(m_max_iteration_count) + ".png"};

            SphereTraceRender();

            m_framebuffer.Screenshot(path / filename);
        }

        m_max_iteration_count = m_benchmark_performance_iteration_count;

        GLuint time_query;
        glGenQueries(1, &time_query);

        std::chrono::high_resolution_clock::time_point time_begin = std::chrono::high_resolution_clock::now();
        glBeginQuery(GL_TIME_ELAPSED, time_query);

        for (unsigned i = 0; i < m_benchmark_performance_number_of_runs; i++) {
            SphereTraceRender();
        }

        glEndQuery(GL_TIME_ELAPSED);

        glFinish();

        std::chrono::high_resolution_clock::time_point time_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> time_diff = time_end - time_begin;
        double ms_taken_chrono = static_cast<double>(time_diff.count()) / static_cast<double>(m_benchmark_performance_number_of_runs);

        GLuint64 ns_taken_gl = 0;
        glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &ns_taken_gl);
        double ms_taken_gl = (static_cast<double>(ns_taken_gl) / 1'000'000.0) / static_cast<double>(m_benchmark_performance_number_of_runs);

        std::string filename{"performance_" + std::to_string(m_benchmark_performance_iteration_count) + ".txt"};

        WriteTimeTaken(path / filename, ms_taken_chrono, ms_taken_gl);
    }

    void App::BenchmarkSingleCone(const std::filesystem::path& base_path, SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType cone_tracing_type, SphereTracingType cone_final_tracing_type) {
        RecompileConeTracer(sdf_scene, cone_tracing_type);
        RecompileConeFinalTracer(sdf_scene, render_mode, cone_final_tracing_type);

        std::filesystem::path path{};
        std::string dir{"cone"};

        switch (cone_tracing_type) {
            case SphereTracingType::NAIVE:    dir += "_naive";    break;
            case SphereTracingType::RELAXED:  dir += "_relaxed";  break;
            case SphereTracingType::ENHANCED: dir += "_enhanced"; break;
            default: break;
        }

        switch (cone_final_tracing_type) {
            case SphereTracingType::NAIVE:    dir += "_naive";    break;
            case SphereTracingType::RELAXED:  dir += "_relaxed";  break;
            case SphereTracingType::ENHANCED: dir += "_enhanced"; break;
            default: break;
        }

        path = base_path / dir;

        std::filesystem::create_directory(path);

        for (m_max_iteration_count = m_benchmark_min_iteration_count; m_max_iteration_count <= m_benchmark_max_iteration_count; m_max_iteration_count += m_benchmark_iteration_count_spacing) {
            std::string filename{"screenshot_" + std::to_string(m_max_iteration_count) + ".png"};

            ConeTraceRender();

            m_framebuffer.Screenshot(path / filename);
        }

        m_max_iteration_count = m_benchmark_performance_iteration_count;

        GLuint time_query;
        glGenQueries(1, &time_query);

        std::chrono::high_resolution_clock::time_point time_begin = std::chrono::high_resolution_clock::now();
        glBeginQuery(GL_TIME_ELAPSED, time_query);

        for (unsigned i = 0; i < m_benchmark_performance_number_of_runs; i++) {
            ConeTraceRender();
        }

        glEndQuery(GL_TIME_ELAPSED);

        glFinish();

        std::chrono::high_resolution_clock::time_point time_end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> time_diff = time_end - time_begin;
        double ms_taken_chrono = static_cast<double>(time_diff.count()) / static_cast<double>(m_benchmark_performance_number_of_runs);

        GLuint64 ns_taken_gl = 0;
        glGetQueryObjectui64v(time_query, GL_QUERY_RESULT, &ns_taken_gl);
        double ms_taken_gl = (static_cast<double>(ns_taken_gl) / 1'000'000.0) / static_cast<double>(m_benchmark_performance_number_of_runs);

        std::string filename{"performance_" + std::to_string(m_benchmark_performance_iteration_count) + ".txt"};

        WriteTimeTaken(path / filename, ms_taken_chrono, ms_taken_gl);
    }

    void App::WriteTimeTaken(const std::filesystem::path& path, double ms_taken_chrono, double ms_taken_gl) {
        std::ofstream output_file{path};
        output_file << ms_taken_chrono << " " << ms_taken_gl;
    }

    void App::RecompileSphereTracer(SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType sphere_tracing_type) {
        m_sphere_trace_shader.Recompile({
            {"SDF_SCENE", std::to_string(static_cast<unsigned>(sdf_scene))},
            {"RENDER_MODE", std::to_string(static_cast<unsigned>(render_mode))},
            {"SPHERE_TRACING_TYPE", std::to_string(static_cast<unsigned>(sphere_tracing_type))}
        });
    }

    void App::RecompileConeTracer(SDFSceneType sdf_scene, SphereTracingType cone_tracing_type) {
        m_cone_trace_shader.Recompile({
            {"SDF_SCENE", std::to_string(static_cast<unsigned>(sdf_scene))},
            {"SPHERE_TRACING_TYPE", std::to_string(static_cast<unsigned>(cone_tracing_type))}
        });
    }

    void App::RecompileConeFinalTracer(SDFSceneType sdf_scene, RenderModeType render_mode, SphereTracingType cone_final_tracing_type) {
        m_cone_trace_final_shader.Recompile({
            {"SDF_SCENE", std::to_string(static_cast<unsigned>(sdf_scene))},
            {"RENDER_MODE", std::to_string(static_cast<unsigned>(render_mode))},
            {"SPHERE_TRACING_TYPE", std::to_string(static_cast<unsigned>(cone_final_tracing_type))}
        });
    }
} // namespace szakdoga::core