#pragma once

#include "Config.hpp"

#include <string>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>

#define VERTEX_ATTRIBUTE 0
#define NORMAL_ATTRIBUTE 1
#define TEXCOORD_ATTRIBUTE 2

#define MODEL_UNIFORM "model"
#define VIEW_UNIFORM "view"
#define PROJECTION_UNIFORM "projection"
#define TEXTURE_UNIFORM "texture"

class Renderer {
	struct Shader {
		GLuint fragmentShader = 0;
		GLuint vertexShader = 0;
		GLuint program = 0;

		GLint uniformModel = -1;
		GLint uniformView = -1;
		GLint uniformProjection = -1;
		GLint uniformTexture = -1;
	};

	Engine& _engine;

	GLFWwindow* _window = nullptr;

	std::vector<Shader> _shaders;

	glm::mat4 _viewMatrix;

	std::string _path;
	glm::uvec2 _windowSize;

	uint64_t _camera = 0;

	void _reshape(int height, int width);

public:
	Renderer(Engine& engine);
	~Renderer();

	void load(int argc, char** argv);
	void update(double dt);

	bool loadMesh(uint64_t* id, const std::string& meshFile);
	bool loadTexture(uint64_t* id, const std::string& textureFile);
	bool loadShader(uint64_t* id, const std::string& vertexShader, const std::string& fragmentShader);

	void setCamera(uint64_t id);

	friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	friend void windowSizeCallback(GLFWwindow* window, int height, int width);
};