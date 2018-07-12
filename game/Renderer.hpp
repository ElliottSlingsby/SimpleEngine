#pragma once

#include "SystemInterface.hpp"
#include "Window.hpp"

#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>

#include <vector>
#include <tuple>
#include <string>
#include <unordered_map>

struct Model {

};

struct Material {

};

class Renderer : public SystemInterface {
public:
	struct Config {
		uint32_t vertexAttribute = 0;
		uint32_t normalAttribute = 1;
		uint32_t texcoordAttribute = 2;
		uint32_t tangentAttribute = 3;
		uint32_t bitangentAttribute = 4;

		std::string modelUniform = "model";
		std::string viewUniform = "view";
		std::string projectionUniform = "projection";
		std::string modelViewUniform = "modelView";
		std::string textureUniform = "texture";
	};

	struct Program {
		GLuint fragmentShader = 0;
		GLuint vertexShader = 0;

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

private:
	Engine& _engine;

	//std::unordered_map<std::string, GLuint> _shaders; // filepath -> shader id
	//std::unordered_map<std::vector<GLuint>, GLuint> _shaderPrograms; // list of compiled shader ids -> program id

	//std::unordered_map<std::string, GLuint> _textures; // filepath -> texture id
	//std::unordered_map<std::string, Mesh> _meshes; // filepath -> vertex array ids

	glm::dmat4 _projectionMatrix;
	double _fov;
	double _zDepth;
	double _width;
	double _height;

	//GLuint _compileShader(const std::string& file);
	//GLuint _createProgram(const std::vector<std::tuple<GLuint, GLuint>>& shaders);

public:
	Renderer(Engine& engine);

	void initiate(int argc, char** argv) override;
	void update(double dt) override;
	void framebufferSize(int width, int height) override;

	void addProgram(uint64_t id, const std::vector<std::tuple<GLuint, std::string>> shaders);
	//void addModel(uint64_t id, const std::string& file);
	//void addTexture(uint64_t id, const std::string& file);
};