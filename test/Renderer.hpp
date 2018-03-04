#pragma once

#include "Config.hpp"

#include <string>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>

class Renderer {
	Engine& _engine;

	GLFWwindow* _window;

	GLuint _fragmentShader;
	GLuint _vertexShader;
	GLuint _program;

	GLuint _attributeVertex;
	GLuint _attributeNormal;
	GLuint _attributeTexcoord;

	GLuint _uniformMatrix;
	GLuint _uniformTexture;

	glm::mat4 _matrix;

	std::string _path;

	glm::uvec2 _windowSize;

public:
	Renderer(Engine& engine);
	~Renderer();

	void load(int argc, char** argv);
	void update(double dt);
};