#include "Texture2D.hpp"

Texture2D::Texture2D(GLsizei width, GLsizei height) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_texture_id);
    glTextureStorage2D(m_texture_id, 1, GL_RGBA32F, width, height);

    glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture2D::~Texture2D() {
    glDeleteTextures(1, &m_texture_id);
}

void Texture2D::Resize(GLsizei width, GLsizei height) {
    glDeleteTextures(1, &m_texture_id);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_texture_id);
    glTextureStorage2D(m_texture_id, 1, GL_RGBA32F, width, height);

    glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

GLuint Texture2D::GetTextureID() {
    return m_texture_id;
}

void Texture2D::Bind(GLuint unit, GLenum access) {
    glBindImageTexture(unit, m_texture_id, 0, GL_FALSE, 0, access, GL_RGBA32F);
}
