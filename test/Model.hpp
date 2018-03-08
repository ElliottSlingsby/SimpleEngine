#pragma once

#include <glad\glad.h>

struct Model {
	GLuint arrayObject = 0;

	GLuint attribBuffer = 0;
	GLuint indexBuffer = 0;

	GLuint texture = 0;

	size_t indexCount = 0;
};