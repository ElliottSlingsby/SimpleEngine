#pragma once

#include "Config.hpp"

#include <string>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>

#define VERTEX_ATTRIBUTE 0
#define NORMAL_ATTRIBUTE 1
#define TEXCOORD_ATTRIBUTE 2

#define VERTEX_SHADER_FILE "vertexShader.glsl"
#define FRAGMENT_SHADER_FILE "fragmentShader.glsl"

#define DATA_FOLDER "data"

class Renderer {
	Engine& _engine;

	GLFWwindow* _window;

	GLuint _fragmentShader;
	GLuint _vertexShader;
	GLuint _program;

	GLint _uniformMatrix;
	GLint _uniformTexture;

	glm::mat4 _matrix;

	std::string _path;
	glm::uvec2 _windowSize;

public:
	Renderer(Engine& engine);
	~Renderer();

	void load(int argc, char** argv);
	void update(double dt);

	void loadMesh(uint64_t* id, const std::string& meshFile);
	void loadTexture(uint64_t* id, const std::string& textureFile);
};