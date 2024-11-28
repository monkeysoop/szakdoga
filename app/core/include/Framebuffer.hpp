#pragma once

#include <GL/glew.h>

#include "Texture2D.hpp"
#include "CompShader.hpp"

class Framebuffer {
public:
    Framebuffer(GLsizei width, GLsizei height);
    ~Framebuffer();

    void Bind();
    void UnBind();
    void Blit();
    void Resize(GLsizei width, GLsizei height);

private:
    GLsizei m_width;
    GLsizei m_height;

    GLuint m_framebuffer_id;
    Texture2D m_target_texture;
};