#include "Framebuffer.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cstring>
#include <cstdint>
#include <memory>
#include <stdexcept>



namespace szakdoga::core {
    Framebuffer::Framebuffer(unsigned width, unsigned height) : 
        m_width{width}, 
        m_height{height}, 
        m_target_texture{width, height, GL_RGBA32F}
    {
        Init();
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

    void Framebuffer::Resize(unsigned width, unsigned height) {
        m_width = width;
        m_height = height;

        m_target_texture.Resize(width, height);

        glDeleteFramebuffers(1, &m_framebuffer_id);

        Init();
    }

    void Framebuffer::Screenshot(const std::filesystem::path& screenshot_path) {
        SDL_Surface* image = SDL_CreateRGBSurfaceWithFormat(0, m_width, m_height, 32, SDL_PIXELFORMAT_RGBA32);
        if (image == nullptr) {
            throw std::runtime_error("Error while creating sdl surface");
        }

        glGetTextureImage(m_target_texture.GetTextureID(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 4 * m_width * m_height, image->pixels);

        FlipSDLSurface(image);

        int result = IMG_SavePNG(image, screenshot_path.c_str());
        if (result != 0) {
            throw std::runtime_error("Error failed to save screenshot path: " + screenshot_path.string());
        }

        SDL_FreeSurface(image);
    }


    void Framebuffer::Init() {
        glCreateFramebuffers(1, &m_framebuffer_id);
        glNamedFramebufferTexture(m_framebuffer_id, GL_COLOR_ATTACHMENT0, m_target_texture.GetTextureID(), 0);
        if (glCheckNamedFramebufferStatus(m_framebuffer_id, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Error while creating framebuffer");
        }
    }

    void Framebuffer::FlipSDLSurface(SDL_Surface* surface) {
        int result = SDL_LockSurface(surface);
        if (result != 0) {
            throw std::runtime_error("Error failed to create lock for sdl surface");
        }

        uint8_t* pixels = static_cast<uint8_t*>(surface->pixels);

        size_t stride = static_cast<size_t>(surface->pitch);
        size_t height = static_cast<size_t>(surface->h);

        std::unique_ptr<uint8_t[]> row_temp{new uint8_t[stride]};

        for (size_t i = 0; i < static_cast<size_t>(height / 2); i++) {
            uint8_t* row_1 = pixels + i * stride;
            uint8_t* row_2 = pixels + (height - 1 - i) * stride;

            std::memcpy(row_temp.get(), row_1, stride);
            std::memcpy(row_1, row_2, stride);
            std::memcpy(row_2, row_temp.get(), stride);
        }

        SDL_UnlockSurface(surface);
    }
} // namespace szakdoga::core