#pragma once

#include "Texture2D.hpp"

#include <GL/glew.h>
#include <SDL2/SDL_surface.h>

#include <filesystem>




namespace szakdoga::core {
    class Framebuffer {
    public:
        Framebuffer(unsigned width, unsigned height);
        ~Framebuffer();

        void Bind();
        void UnBind();
        void Blit();
        void Resize(unsigned width, unsigned height);
        void Screenshot(const std::filesystem::path& screenshot_path);

    private:
        unsigned m_width;
        unsigned m_height;

        GLuint m_framebuffer_id;
        Texture2D m_target_texture;

    private:
        void Init();
        void FlipSDLSurface(SDL_Surface* surface);
    };
} // namespace szakdoga::core