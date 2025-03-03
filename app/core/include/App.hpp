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

enum class SphereTracingType : unsigned {
    NAIVE = 0,
    RELAXED = 1,
    ENHANCED = 2,
    CONE = 3,
};


class App {
public:
    App(unsigned width, unsigned height);
    ~App();

    void Update(float elapsed_time_in_seconds, float delta_time_in_seconds);
    void Render();
    void RenderImGui();

    void KeyboardDown(const SDL_KeyboardEvent& key);
    void KeyboardUp(const SDL_KeyboardEvent& key);
    void MouseMove(const SDL_MouseMotionEvent& mouse);
    void MouseWheel(const SDL_MouseWheelEvent& wheel);
    void Resize(unsigned width, unsigned height);

private:
    unsigned m_width;
    unsigned m_height;

    Camera m_camera;
    CameraManipulator m_camera_manipulator;

    Framebuffer m_framebuffer;

    CompShader m_naive_shader;
    CompShader m_cone_shader;
    CompShader m_cone_final_shader;
    CompShader m_cone_precompute_shader;

    Texture2D m_cone_distance_iteration_texture_1;
    Texture2D m_cone_distance_iteration_texture_2;

    unsigned m_initial_cone_size;
    Texture2D m_cone_precomputed_texture;

    Skybox m_skybox;

    SphereTracingType m_render_mode;
    bool m_show_iterations;

    float m_time_in_seconds;
    float m_epsilon;
    float m_max_distance;
    unsigned m_max_iteration_count;

private:
    void SphereTraceRender();
    void ConeTraceRender();
    void PrecomputeCones();
    unsigned DivideAndRoundUp(unsigned number, unsigned divisor);
};