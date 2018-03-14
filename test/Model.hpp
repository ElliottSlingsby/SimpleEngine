#pragma once

#include <glad\glad.h>

struct Model {
	GLuint arrayObject = 0;

	GLuint attribBuffer = 0;

	GLuint texture = 0;

	GLsizei indexCount = 0;

	bool hasShader = false;
	uint32_t shader;
};