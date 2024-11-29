#include <imgui.h>

#include "App.hpp"
#include "SDL_GLDebugMessageCallback.h"

#include <iostream>
#include <SDL2/SDL_image.h>


App::App(GLsizei width, GLsizei height) : 
    m_width{width}, 
    m_height{height}, 
    m_camera{}, 
    m_camera_manipulator{}, 
    m_framebuffer{width, height}, 
    m_test_shader{"assets/test.comp"}, 
    m_naive_shader{"assets/naive_sphere_tracing.comp"},
    m_skybox{} 
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

    m_camera.SetView(glm::vec3(2, 2, 2),  // From where we look at the scene - eye
                     glm::vec3(0, 0, 0),   // Which point of the scene we are looking at - at
                     glm::vec3(0, 1, 0)    // Upwards direction - up
    );

    m_camera_manipulator.SetCamera(&m_camera);

    m_skybox.InitTexture();
}

App::~App() {}

void App::Update(const SUpdateInfo& updateInfo) {
    m_camera_manipulator.Update(updateInfo.DeltaTimeInSec);
}


void App::Render() {
    m_framebuffer.Bind();
    m_naive_shader.Use();
    glUniformMatrix4fv(m_naive_shader.ul("inv_view_proj_mat"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_camera.GetViewProj())));
    glUniform3fv(m_naive_shader.ul("position"), 1, glm::value_ptr(m_camera.GetEye()));
    glUniform1f(m_naive_shader.ul("width"), static_cast<GLfloat>(m_width));
    glUniform1f(m_naive_shader.ul("height"), static_cast<GLfloat>(m_height));
    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox.skyboxTextureID);
    glUniform1i(m_naive_shader.ul("skyboxTexture"), 0);
    m_naive_shader.Dispatch(((m_width + 15) / 16), ((m_height + 15) / 16), 1);
    m_naive_shader.Barrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    m_framebuffer.UnBind();
    m_framebuffer.Blit();
}

void App::RenderImGui() {
    ImGui::Begin("Demo window");
    ImGui::Button("Hello!");
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
    m_camera.SetAspect(static_cast<float>(width) / static_cast<float>(width));
}