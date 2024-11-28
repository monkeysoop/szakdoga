#pragma once
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

class Skybox {
public:
	Skybox();
	~Skybox();

	void InitTexture();
	void CleanTexture();

	GLuint skyboxTextureID = 0;
};