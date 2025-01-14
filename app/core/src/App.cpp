#include <imgui.h>

#include "App.hpp"
#include "SDL_GLDebugMessageCallback.h"

#include <iostream>


App::App(GLsizei width, GLsizei height) : 
    m_width{width}, 
    m_height{height}, 
    m_camera{}, 
    m_camera_manipulator{}, 
    m_framebuffer{width, height}, 
    m_naive_shader{"assets/naive_sphere_tracing.comp"},
    m_cone_shader{"assets/cone_tracer.comp"},
    m_cone_final_shader{"assets/cone_tracer_final_stage.comp"},
    m_cone_distance_texture_1{width, height, GL_R32F},
    m_cone_distance_texture_2{width, height, GL_R32F},
    m_cone_distance_texture_blank{width, height, GL_R32F},
    m_skybox{},
    m_render_mode{true},
    m_time_in_seconds{0.0f},
    m_epsilon{0.001f},
    m_max_distance{1000.0f},
    m_max_iteration_count{100},
    m_initial_cone_size{8}
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
}

App::~App() {}

void App::Update(float elapsed_time_in_seconds, float delta_time_in_seconds) {
    m_time_in_seconds = elapsed_time_in_seconds;
    m_camera_manipulator.Update(delta_time_in_seconds);
}


void App::Render() {
    if (m_render_mode) {
        m_cone_shader.Use();
        int cone_size = m_initial_cone_size;
        bool t = true;
        while (cone_size > 1) {
            //std::cout << "cone size: " << cone_size << " t: " << t << std::endl;
            glUniformMatrix4fv(m_cone_shader.ul("inv_view_proj_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewProj())));
            glUniform3fv(m_cone_shader.ul("position"), 1, glm::value_ptr(m_camera.GetEye()));
            glUniform1f(m_cone_shader.ul("width"), static_cast<GLfloat>(m_width));
            glUniform1f(m_cone_shader.ul("height"), static_cast<GLfloat>(m_height));

            glUniform1f(m_cone_shader.ul("time_in_seconds"), static_cast<GLfloat>(m_time_in_seconds));
            glUniform1f(m_cone_shader.ul("epsilon"), static_cast<GLfloat>(m_epsilon));
            glUniform1f(m_cone_shader.ul("max_distance"), static_cast<GLfloat>(m_max_distance));
            glUniform1i(m_cone_shader.ul("max_iteration_count"), static_cast<GLint>(m_max_iteration_count));

            glUniform1i(m_cone_shader.ul("cone_size"), static_cast<GLint>(cone_size));

            if (cone_size == m_initial_cone_size) {
                m_cone_distance_texture_blank.Bind(0, GL_READ_ONLY);
                if (t) {
                    m_cone_distance_texture_2.Bind(1, GL_WRITE_ONLY);
                } else {
                    m_cone_distance_texture_1.Bind(1, GL_WRITE_ONLY);
                }
            } else {
                if (t) {
                    m_cone_distance_texture_1.Bind(0, GL_READ_ONLY);
                    m_cone_distance_texture_2.Bind(1, GL_WRITE_ONLY);
                } else {
                    m_cone_distance_texture_2.Bind(0, GL_READ_ONLY);
                    m_cone_distance_texture_1.Bind(1, GL_WRITE_ONLY);
                }
            }

            m_cone_shader.Dispatch(((m_width / cone_size) / 16), ((m_height / cone_size) / 16), 1);
            m_cone_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            cone_size /= 2;
            t = !t;
        }

        m_framebuffer.Bind();
        m_cone_final_shader.Use();
        if (t) {
            m_cone_distance_texture_1.Bind(1, GL_READ_ONLY);
        } else {
            m_cone_distance_texture_2.Bind(1, GL_READ_ONLY);
        }
        glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.GetTextureID());
        glUniform1i(m_cone_final_shader.ul("skyboxTexture"), 0);

        glUniformMatrix4fv(m_cone_final_shader.ul("inv_view_proj_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewProj())));
        glUniform3fv(m_cone_final_shader.ul("position"), 1, glm::value_ptr(m_camera.GetEye()));
        glUniform1f(m_cone_final_shader.ul("width"), static_cast<GLfloat>(m_width));
        glUniform1f(m_cone_final_shader.ul("height"), static_cast<GLfloat>(m_height));
        
        glUniform1f(m_cone_final_shader.ul("time_in_seconds"), static_cast<GLfloat>(m_time_in_seconds));
        glUniform1f(m_cone_final_shader.ul("epsilon"), static_cast<GLfloat>(m_epsilon));
        glUniform1f(m_cone_final_shader.ul("max_distance"), static_cast<GLfloat>(m_max_distance));
        glUniform1i(m_cone_final_shader.ul("max_iteration_count"), static_cast<GLint>(m_max_iteration_count));

        m_cone_final_shader.Dispatch((m_width / 16), (m_height / 16), 1);
        m_cone_final_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        m_framebuffer.UnBind();
        m_framebuffer.Blit();

    } else {
        m_framebuffer.Bind();
        m_naive_shader.Use();
        glActiveTexture(GL_TEXTURE0);
	    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.GetTextureID());
        glUniform1i(m_naive_shader.ul("skyboxTexture"), 0);
        
        glUniformMatrix4fv(m_naive_shader.ul("inv_view_proj_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewProj())));
        glUniform3fv(m_naive_shader.ul("position"), 1, glm::value_ptr(m_camera.GetEye()));
        glUniform1f(m_naive_shader.ul("width"), static_cast<GLfloat>(m_width));
        glUniform1f(m_naive_shader.ul("height"), static_cast<GLfloat>(m_height));

        glUniform1f(m_naive_shader.ul("time_in_seconds"), static_cast<GLfloat>(m_time_in_seconds));
        glUniform1f(m_naive_shader.ul("epsilon"), static_cast<GLfloat>(m_epsilon));
        glUniform1f(m_naive_shader.ul("max_distance"), static_cast<GLfloat>(m_max_distance));
        glUniform1i(m_naive_shader.ul("max_iteration_count"), static_cast<GLint>(m_max_iteration_count));
        m_naive_shader.Dispatch((m_width / 16), (m_height / 16), 1);
        m_naive_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        m_framebuffer.UnBind();
        m_framebuffer.Blit();
    }
}

void App::RenderImGui() {
    if (ImGui::Begin("Settings")) {
		ImGui::Checkbox("use cone tracing", &m_render_mode);
		ImGui::SliderFloat("epsilon", &m_epsilon, 0.000001f, 0.01f);
		ImGui::SliderFloat("max distance", &m_max_distance, 0.0f, 10000.0f);
		ImGui::SliderInt("max iteration count", &m_max_iteration_count, 0, 1000);
        if (ImGui::Button("decrease cone size") && m_initial_cone_size > 1) {
            m_initial_cone_size /= 2;
        }
        ImGui::Text("cone size: %d", m_initial_cone_size);
        if (ImGui::Button("increase cone size") && m_initial_cone_size < 256) {
            m_initial_cone_size *= 2;
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
}

void App::Resize(GLsizei width, GLsizei height) {
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
    m_framebuffer.Resize(width, height);
    m_camera.SetAspect(static_cast<float>(width) / static_cast<float>(height));
    m_cone_distance_texture_1.Resize(width, height);
    m_cone_distance_texture_2.Resize(width, height);
}