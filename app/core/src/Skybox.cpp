#include "Skybox.hpp"

#include <SDL2/SDL_image.h>
#include <array>
#include <string>

Skybox::Skybox() {
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_texture_id);
    glTextureStorage2D(m_texture_id, 1, GL_RGBA8, 900, 900);

    std::array<std::string, 6> filenames = {
        "assets/skybox_xpos.png", 
        "assets/skybox_xneg.png", 
        "assets/skybox_ypos.png",
        "assets/skybox_yneg.png", 
        "assets/skybox_zpos.png", 
        "assets/skybox_zneg.png"
    };

    for (int i = 0; i < 6; i++) {
        SDL_Surface* loaded_img = IMG_Load(filenames[i].c_str());

        if (loaded_img == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "[TextureFromFile] Error while loading texture: %s", filenames[i].c_str());
        }

        SDL_Surface* formattedSurf = SDL_ConvertSurfaceFormat(loaded_img, ((SDL_BYTEORDER == SDL_LIL_ENDIAN) ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_RGBA8888), 0);
        SDL_FreeSurface(loaded_img);
        if (formattedSurf == nullptr) {
            SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "[TextureFromFile] Error while processing texture");
            return;
        }

        glTextureSubImage3D(m_texture_id, 0, 0, 0, i, formattedSurf->w, formattedSurf->h, 1, GL_RGBA, GL_UNSIGNED_BYTE, formattedSurf->pixels);
        SDL_FreeSurface(formattedSurf);
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
