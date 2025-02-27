#pragma once

#include <GL/glew.h>

#include <SDL2/SDL_surface.h>

#include <filesystem>

#include "Texture2D.hpp"

class Framebuffer {
public:
    Framebuffer(GLsizei width, GLsizei height);
    ~Framebuffer();

    void Bind();
    void UnBind();
    void Blit();
    void Resize(GLsizei width, GLsizei height);
    void Screenshot(const std::filesystem::path& screenshot_path);

private:
    GLsizei m_width;
    GLsizei m_height;

    GLuint m_framebuffer_id;
    Texture2D m_target_texture;

    void Init();
    void FlipSDLSurface(SDL_Surface* surface);
};