#pragma once

#include <GL/glew.h>

class Skybox {
public:
	Skybox();
	~Skybox();

	GLuint GetTextureID();

private:
    GLuint m_texture_id;
};