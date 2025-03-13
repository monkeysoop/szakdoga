#pragma once

#include <GL/glew.h>



namespace szakdoga::core {
    class Skybox {
    public:
        Skybox();
        ~Skybox();

        GLuint GetTextureID();

    private:
        GLuint m_texture_id;
    };
}  // namespace szakdoga::core