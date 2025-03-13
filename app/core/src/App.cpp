#include "App.hpp"
#include "SDL_GLDebugMessageCallback.h"

#include <imgui.h>

#include <filesystem>
#include <iostream>
#include <cmath>



namespace szakdoga::core {
    const unsigned LOCAL_WORKGROUP_SIZE_X = 8;
    const unsigned LOCAL_WORKGROUP_SIZE_Y = 8;

    App::App(unsigned width, unsigned height) : 
        m_width{width}, 
        m_height{height}, 
        m_camera{}, 
        m_camera_manipulator{}, 
        m_framebuffer{width, height}, 
        m_sphere_trace_shader{std::filesystem::path{"assets"} / "sphere_tracer.comp", {std::filesystem::path{"assets"} / "sdf.include"}},
        m_cone_trace_shader{std::filesystem::path{"assets"} / "cone_tracer.comp", {std::filesystem::path{"assets"} / "sdf.include"}},
        m_cone_trace_final_shader{std::filesystem::path{"assets"} / "cone_tracer_final_stage.comp", {std::filesystem::path{"assets"} / "sdf.include"}},
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
        m_render_mode{SphereTracingType::NAIVE},
        m_show_iterations{false},
        m_time_in_seconds{0.0f},
        m_epsilon{0.001f},
        m_max_distance{1000.0f},
        m_max_iteration_count{100}
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
        if (m_render_mode == SphereTracingType::CONE) {
            ConeTraceRender();
        } else {
            SphereTraceRender();
        }
    }

    void App::RenderImGui() {
        if (ImGui::Begin("Settings")) {
            int mode = static_cast<int>(m_render_mode);
            ImGui::RadioButton("naive", &mode, static_cast<int>(SphereTracingType::NAIVE));
            ImGui::RadioButton("relaxed", &mode, static_cast<int>(SphereTracingType::RELAXED));
            ImGui::RadioButton("enhanced", &mode, static_cast<int>(SphereTracingType::ENHANCED));
            ImGui::RadioButton("cone", &mode, static_cast<int>(SphereTracingType::CONE));
            m_render_mode = static_cast<SphereTracingType>(mode);
            ImGui::Checkbox("show iteration counts", &m_show_iterations);
            ImGui::SliderFloat("epsilon", &m_epsilon, 0.000001f, 0.01f);
            ImGui::SliderFloat("max distance", &m_max_distance, 0.0f, 10000.0f);
            int iter_count = static_cast<int>(m_max_iteration_count);
            ImGui::SliderInt("max iteration count", &iter_count, 0, 1000);
            m_max_iteration_count = static_cast<unsigned>(iter_count);
            if (ImGui::Button("decrease cone size") && m_initial_cone_size > 1) {
                m_initial_cone_size /= 2;
                m_cone_trace_precomputed_texture.Resize(
                    static_cast<unsigned>((m_width + m_initial_cone_size - 1) / 2), 
                    static_cast<unsigned>((m_height + m_initial_cone_size - 1) / 2), 
                    static_cast<unsigned>(std::log2(m_initial_cone_size))
                );
                PrecomputeCones();
            }
            ImGui::Text("cone size: %d", m_initial_cone_size);
            if (ImGui::Button("increase cone size") && m_initial_cone_size < 256) {
                m_initial_cone_size *= 2;
                m_cone_trace_precomputed_texture.Resize(
                    static_cast<unsigned>((m_width + m_initial_cone_size - 1) / 2), 
                    static_cast<unsigned>((m_height + m_initial_cone_size - 1) / 2), 
                    static_cast<unsigned>(std::log2(m_initial_cone_size))
                );
                PrecomputeCones();
            }
            if (ImGui::Button("take screenhsot")) {
                m_framebuffer.Screenshot("screenshot.png");
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
        glUniform1ui(m_sphere_trace_shader.ul("u_max_iteration_count"), static_cast<GLuint>(m_max_iteration_count));

        glUniform1i(m_sphere_trace_shader.ul("u_show_iterations"), static_cast<GLint>(m_show_iterations));

        glUniform1ui(m_sphere_trace_shader.ul("u_render_mode"), static_cast<GLuint>(m_render_mode));

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
            glUniform1f(m_cone_trace_shader.ul("u_epsilon"), static_cast<GLfloat>(m_epsilon));
            glUniform1f(m_cone_trace_shader.ul("u_max_distance"), static_cast<GLfloat>(m_max_distance));
            glUniform1f(m_cone_trace_shader.ul("u_max_iteration_count"), static_cast<GLfloat>(m_max_iteration_count));

            glUniform1i(m_cone_trace_shader.ul("u_first_pass"), static_cast<GLint>(first_pass));

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
        glUniform1i(m_cone_trace_final_shader.ul("u_show_iterations"), static_cast<GLint>(m_show_iterations));

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
        float old_time = m_time_in_seconds;
        unsigned old_iter_count = m_max_iteration_count;
        SphereTracingType old_mode = m_render_mode;
        bool old_show = m_show_iterations;

        m_time_in_seconds = 0.0;


        std::filesystem::path visual_path{std::filesystem::path{"benchmarks"} / "visual"};

        m_show_iterations = false;

        m_render_mode = SphereTracingType::NAIVE;
        m_max_iteration_count = 1000;
        SphereTraceRender();
        m_framebuffer.Screenshot(visual_path / "screenshot_baseline.png");

        for (m_max_iteration_count = 10; m_max_iteration_count <= 100; m_max_iteration_count += 10) {
            std::string filename{"screenshot_" + std::to_string(m_max_iteration_count) + ".png"};

            m_render_mode = SphereTracingType::NAIVE;
            SphereTraceRender();
            m_framebuffer.Screenshot(visual_path / "naive" / filename);

            m_render_mode = SphereTracingType::RELAXED;
            SphereTraceRender();
            m_framebuffer.Screenshot(visual_path / "relaxed" / filename);

            m_render_mode = SphereTracingType::ENHANCED;
            SphereTraceRender();
            m_framebuffer.Screenshot(visual_path / "enhanced" / filename);

            m_render_mode = SphereTracingType::CONE;
            ConeTraceRender();
            m_framebuffer.Screenshot(visual_path / "cone" / filename);
        }


        std::filesystem::path performance_path{std::filesystem::path{"benchmarks"} / "performance"};

        m_show_iterations = true;

        for (m_max_iteration_count = 10; m_max_iteration_count <= 100; m_max_iteration_count += 10) {
            std::string filename{"screenshot_" + std::to_string(m_max_iteration_count) + ".png"};

            m_render_mode = SphereTracingType::NAIVE;
            SphereTraceRender();
            m_framebuffer.Screenshot(performance_path / "naive" / filename);

            m_render_mode = SphereTracingType::RELAXED;
            SphereTraceRender();
            m_framebuffer.Screenshot(performance_path / "relaxed" / filename);

            m_render_mode = SphereTracingType::ENHANCED;
            SphereTraceRender();
            m_framebuffer.Screenshot(performance_path / "enhanced" / filename);

            m_render_mode = SphereTracingType::CONE;
            ConeTraceRender();
            m_framebuffer.Screenshot(performance_path / "cone" / filename);
        }


        m_time_in_seconds = old_time;
        m_max_iteration_count = old_iter_count;
        m_render_mode = old_mode;
        m_show_iterations = old_show;
    }
} // namespace szakdoga::core