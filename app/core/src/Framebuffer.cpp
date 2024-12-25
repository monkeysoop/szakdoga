#include "Framebuffer.hpp"

#include <SDL2/SDL.h>

Framebuffer::Framebuffer(GLsizei width, GLsizei height)
    : m_width{width}, m_height{height}, m_target_texture{width, height, GL_RGBA32F} {
    glCreateFramebuffers(1, &m_framebuffer_id);
    glNamedFramebufferTexture(m_framebuffer_id, GL_COLOR_ATTACHMENT0, m_target_texture.GetTextureID(), 0);
    if (glCheckNamedFramebufferStatus(m_framebuffer_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error while creating framebuffer");
    }
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_framebuffer_id);
}

void Framebuffer::Bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer_id);
    m_target_texture.Bind(0, GL_WRITE_ONLY);
}

void Framebuffer::UnBind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Blit() {
    glBlitNamedFramebuffer(m_framebuffer_id, 0, 0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void Framebuffer::Resize(GLsizei width, GLsizei height) {
    m_width = width;
    m_height = height;
    m_target_texture.Resize(width, height);

    glDeleteFramebuffers(1, &m_framebuffer_id);

    glCreateFramebuffers(1, &m_framebuffer_id);
    glNamedFramebufferTexture(m_framebuffer_id, GL_COLOR_ATTACHMENT0, m_target_texture.GetTextureID(), 0);
    if (glCheckNamedFramebufferStatus(m_framebuffer_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "Error while creating framebuffer");
    }
}