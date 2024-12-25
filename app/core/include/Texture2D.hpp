#pragma once

#include <GL/glew.h>

class Texture2D {
public:
    Texture2D(GLsizei width, GLsizei height, GLenum format);
    ~Texture2D();

    void Resize(GLsizei width, GLsizei height);
    GLuint GetTextureID();
    void Bind(GLuint unit, GLenum access);

private:
    GLuint m_texture_id;
    GLenum m_format;
};