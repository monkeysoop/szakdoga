#pragma once

#include <GL/glew.h>

class Texture2D {
public:
    Texture2D(GLsizei width, GLsizei height, GLenum format, GLsizei levels=1);
    ~Texture2D();

    void Resize(GLsizei width, GLsizei height);
    void ChangeLevels(GLsizei levels);
    GLuint GetTextureID();
    void Bind(GLuint unit, GLenum access, GLint level=0);

private:
    GLuint m_texture_id;

    GLsizei m_width;
    GLsizei m_height;
    
    GLenum m_format;
    GLsizei m_levels;

    void Init();
};