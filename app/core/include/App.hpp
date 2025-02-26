#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "Camera.hpp"
#include "CameraManipulator.hpp"
#include "Framebuffer.hpp"
#include "CompShader.hpp"
#include "Skybox.hpp"
#include "Texture2D.hpp"


class App {
public:
    App(GLsizei width, GLsizei height);
    ~App();

    void Update(float elapsed_time_in_seconds, float delta_time_in_seconds);
    void Render();
    void RenderImGui();

    void KeyboardDown(const SDL_KeyboardEvent& key);
    void KeyboardUp(const SDL_KeyboardEvent& key);
    void MouseMove(const SDL_MouseMotionEvent& mouse);
    void MouseWheel(const SDL_MouseWheelEvent& wheel);
    void Resize(GLsizei width, GLsizei height);

private:
    GLsizei m_width;
    GLsizei m_height;

    Camera m_camera;
    CameraManipulator m_camera_manipulator;

    Framebuffer m_framebuffer;

    CompShader m_naive_shader;
    CompShader m_cone_shader;
    CompShader m_cone_final_shader;
    CompShader m_cone_precompute_shader;

    Texture2D m_cone_distance_iteration_texture_1;
    Texture2D m_cone_distance_iteration_texture_2;

    int m_initial_cone_size;
    Texture2D m_cone_precomputed_texture;

    Skybox m_skybox;

    bool m_render_mode;
    bool m_show_iterations;

    float m_time_in_seconds;
    float m_epsilon;
    float m_max_distance;
    int m_max_iteration_count;

private:
    void NaiveRender();
    void ConeRender();
    void PrecomputeCones();
    GLuint DivideAndRoundUp(GLuint number, GLuint divisor);
};