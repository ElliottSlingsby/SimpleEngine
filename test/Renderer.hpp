#pragma once

#include "Config.hpp"
#include "RenderCoords.hpp"

#include <string>
#include <unordered_map>

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>

#include <assimp\scene.h>

#define VERTEX_ATTRIBUTE 0
#define NORMAL_ATTRIBUTE 1
#define TEXCOORD_ATTRIBUTE 2
//#define TANGENT_ATTRIBUTE 3
//#define BITANGENT_ATTRIBUTE 4

#define MODEL_UNIFORM "model"
#define VIEW_UNIFORM "view"
#define PROJECTION_UNIFORM "projection"
#define MODELVIEW_UNIFORM "modelView"
#define TEXTURE_UNIFORM "texture"

class Renderer {
	struct Program {
		GLuint fragmentShader = 0;
		GLuint vertexShader = 0;
		GLuint program = 0;

		GLint uniformModel = -1;
		GLint uniformView = -1;
		GLint uniformProjection = -1;
		GLint uniformModelView = -1;
		GLint uniformTexture = -1;
	};

	struct Mesh {
		GLuint arrayObject = 0;
		GLuint attribBuffer = 0;
		GLsizei indexCount = 0;
	};

	Engine& _engine;

	GLFWwindow* _window = nullptr;

	std::vector<Program> _programs;

	glm::mat4 _projectionMatrix;

	std::string _path;
	glm::uvec2 _windowSize;

	uint64_t _camera = 0;

	std::unordered_map<std::string, uint32_t> _shaders;
	std::unordered_map<std::string, GLuint> _textures;
	std::unordered_map<std::string, Mesh> _meshes;

	void _reshape(int height, int width);

	void _extract(uint64_t parent, const aiScene* scene, const aiNode* node);

public:
	Renderer(Engine& engine);
	~Renderer();

	void load(int argc, char** argv);
	void update(double dt);

	bool addScene(uint64_t* id, const std::string& sceneFile);

	bool addMesh(uint64_t* id, const std::string& meshFile);
	bool addTexture(uint64_t* id, const std::string& textureFile);
	bool addShader(uint64_t* id, const std::string& vertexShader, const std::string& fragmentShader);

	void setCamera(uint64_t id);

	void lockCursor(bool lock);

	friend void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	friend void cursorCallback(GLFWwindow* window, double x, double y);
	friend void windowSizeCallback(GLFWwindow* window, int height, int width);
};