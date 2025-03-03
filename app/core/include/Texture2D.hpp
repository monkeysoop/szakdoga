#pragma once

#include <GL/glew.h>

class Texture2D {
public:
    Texture2D(unsigned width, unsigned height, GLenum format, unsigned levels=1);
    ~Texture2D();

    void Resize(unsigned width, unsigned height);
    void Resize(unsigned width, unsigned height, unsigned levels);
    GLuint GetTextureID();
    void Bind(GLuint unit, GLenum access, unsigned level=0);

private:
    GLuint m_texture_id;

    unsigned m_width;
    unsigned m_height;
    
    GLenum m_format;
    unsigned m_levels;

    void Init();
};