#pragma once

#include <glad\glad.h>

struct Model {
	GLuint vertexBuffer = GL_NONE;
	GLuint indexBuffer = GL_NONE;
	GLuint texture = GL_NONE;
};