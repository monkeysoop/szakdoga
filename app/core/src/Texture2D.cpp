#include "Texture2D.hpp"

Texture2D::Texture2D(unsigned width, unsigned height, GLenum format, unsigned levels) : 
    m_width{width},
    m_height{height},
    m_format{format}, 
    m_levels{levels} 
{
    Init();
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &m_texture_id);
}

void Texture2D::Resize(unsigned width, unsigned height) {
    glDeleteTextures(1, &m_texture_id);

    m_width = width;
    m_height = height;

    Init();
}

void Texture2D::Resize(unsigned width, unsigned height, unsigned levels) {
    glDeleteTextures(1, &m_texture_id);

    m_width = width;
    m_height = height;

    m_levels = levels;

    Init();
}

GLuint Texture2D::GetTextureID() {
    return m_texture_id;
}

void Texture2D::Bind(GLuint unit, GLenum access, unsigned level) {
    glBindImageTexture(unit, m_texture_id, level, GL_FALSE, 0, access, m_format);
}

void Texture2D::Init() {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_texture_id);
    glTextureStorage2D(m_texture_id, m_levels, m_format, m_width, m_height);

    glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_texture_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_texture_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
