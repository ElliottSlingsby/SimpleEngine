#pragma once

#include <glad\glad.h>

struct Model {
	GLuint arrayObject = GL_NONE;

	GLuint attribBuffer = GL_NONE;
	GLuint indexBuffer = GL_NONE;

	size_t indexCount = 0;
};