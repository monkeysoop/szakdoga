#pragma once

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "Camera.hpp"
#include "CameraManipulator.hpp"
#include "Framebuffer.hpp"
#include "Skybox.hpp"


struct SUpdateInfo {
    float ElapsedTimeInSec = 0.0f;  // Elapsed time since start of the program
    float DeltaTimeInSec = 0.0f;    // Elapsed time since last update
};

class App {
public:
    App(GLsizei width, GLsizei height);
    ~App();

    void Update(const SUpdateInfo& updateInfo);
    void Render();
    void RenderImGui();

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
    CompShader m_test_shader;
    CompShader m_naive_shader;

    Skybox m_skybox;
};