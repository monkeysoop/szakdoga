#include "Skybox.hpp"

#include <SDL2/SDL_image.h>

#include <vector>
#include <filesystem>
#include <algorithm>



namespace szakdoga::core {
    Skybox::Skybox() {
        glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_texture_id);

        std::vector<std::filesystem::path> filenames{
            std::filesystem::path{"assets"} / "skybox_xpos.png", 
            std::filesystem::path{"assets"} / "skybox_xneg.png", 
            std::filesystem::path{"assets"} / "skybox_ypos.png", 
            std::filesystem::path{"assets"} / "skybox_yneg.png", 
            std::filesystem::path{"assets"} / "skybox_zpos.png", 
            std::filesystem::path{"assets"} / "skybox_zneg.png" 
        };

        std::vector<SDL_Surface*> surfaces{};

        int max_width = 0;
        int max_height = 0;

        for (const std::filesystem::path& filename : filenames) {
            SDL_Surface* loaded_surface = IMG_Load(filename.c_str());

            if (loaded_surface == nullptr) {
                SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "[TextureFromFile] Error while loading texture: %s", filename.c_str());
            }

            SDL_Surface* formatted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(loaded_surface);
            if (formatted_surface == nullptr) {
                SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "[TextureFromFile] Error while processing texture");
                return;
            }

            surfaces.push_back(formatted_surface);
            max_width = std::max(max_width, formatted_surface->w);
            max_height = std::max(max_height, formatted_surface->h);
        }


        glTextureStorage2D(m_texture_id, 1, GL_RGBA8, max_width, max_height);

        for (size_t index = 0; index < surfaces.size(); index++) {
            glTextureSubImage3D(m_texture_id, 0, 0, 0, index, surfaces[index]->w, surfaces[index]->h, 1, GL_RGBA, GL_UNSIGNED_BYTE, surfaces[index]->pixels);
            SDL_FreeSurface(surfaces[index]);
        }

        glTextureParameteri(m_texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_texture_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    Skybox::~Skybox() {
        glDeleteTextures(1, &m_texture_id);
    }

    GLuint Skybox::GetTextureID() {
        return m_texture_id;
    }
} // namespace szakdoga::core