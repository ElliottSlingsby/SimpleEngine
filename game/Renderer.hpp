#pragma once

#include "SystemInterface.hpp"

#include <glad\glad.h>

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
	struct ShaderVariables {
		uint32_t vertexAttrLoc = 0;
		uint32_t normalAttrLoc = 1;
		uint32_t texcoordAttrLoc = 2;
		uint32_t tangentAttrLoc = 3;
		uint32_t bitangentAttrLoc = 4;
		uint32_t colourAttrLoc = 5;
		uint32_t boneWeightsAttrLoc = 6;
		uint32_t boneIndexesAttrLoc = 7;

		std::string modelUnifName = "model";
		std::string viewUnifName = "view";
		std::string projectionUnifName = "projection";
		std::string modelViewUnifName = "modelView";
		std::string textureUnifName = "texture";
		std::string bonesUnifName = "bones";
	};

	struct ShapeConfig {
		float verticalFov;
		float zDepth;
		glm::uvec2 resolution;
	};

	struct Program {
		GLuint fragmentShader = 0;
		GLuint vertexShader = 0;

		GLint modelUnifLoc = -1;
		GLint viewUnifLoc = -1;
		GLint projectionUnifLoc = -1;
		GLint modelViewUnifLoc = -1;
		GLint textureUnifLoc = -1;
		GLint bonesUnifLoc = -1;
	};

	struct Mesh {
		GLuint arrayObject = 0;
		GLuint attribBuffer = 0;
		GLsizei indexCount = 0;
	};

private:
	Engine& _engine;

	const ShaderVariables _shaderVariables;

	//std::unordered_map<std::string, GLuint> _shaders; // filepath -> shader id
	//std::unordered_map<std::vector<GLuint>, GLuint> _shaderPrograms; // list of compiled shader ids -> program id

	//std::unordered_map<std::string, GLuint> _textures; // filepath -> texture id
	//std::unordered_map<std::string, Mesh> _meshes; // filepath -> vertex array ids

	glm::mat4 _projectionMatrix;
	float _verticalFov;
	float _zDepth;
	glm::vec2 _size;

	//GLuint _compileShader(const std::string& file);
	//GLuint _createProgram(const std::vector<std::tuple<GLuint, GLuint>>& shaders);

public:
	Renderer(Engine& engine, const ShaderVariables& shaderVariables = ShaderVariables());

	void initiate(const std::vector<std::string>& args) override;
	void update(double dt) override;
	void framebufferSize(glm::uvec3 size) override;
	void textureLoaded(uint64_t id, const std::string& file, const TextureData* textureData) override;
	void meshLoaded(uint64_t id, const std::string& file, const MeshData* meshData) override;

	void setShape(const ShapeConfig& config);

	void loadProgram(const std::vector<std::tuple<GLuint, std::string>>& shaders, uint64_t id = 0, bool reload = false);
};