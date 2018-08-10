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
	GLuint arrayObject = 0;
	GLuint vertexBuffer = 0;
	GLuint indexBuffer = 0;
	uint32_t indexCount = 0;

	GLuint textureBuffer = 0;

	uint32_t program = 0;
};

class Renderer : public SystemInterface {
public:
	struct ConstructorInfo {
		uint32_t positionAttrLoc = 0;
		uint32_t normalAttrLoc = 1;
		uint32_t texcoordAttrLoc = 2;
		uint32_t tangentAttrLoc = 3;
		uint32_t bitangentAttrLoc = 4;
		uint32_t colourAttrLoc = 5;
		//uint32_t boneWeightsAttrLoc = 6;
		//uint32_t boneIndexesAttrLoc = 7;

		std::string modelUnifName = "model";
		std::string viewUnifName = "view";
		std::string projectionUnifName = "projection";
		std::string modelViewUnifName = "modelView";
		std::string textureUnifName = "texture";
		//std::string bonesUnifName = "bones";
	};

	struct ShapeInfo {
		float verticalFov;
		float zDepth;
	};

	struct ShaderProgram {
		GLuint program = 0;

		GLint modelUnifLoc = -1;
		GLint viewUnifLoc = -1;
		GLint projectionUnifLoc = -1;
		GLint modelViewUnifLoc = -1;
		GLint textureUnifLoc = -1;
		GLint bonesUnifLoc = -1;
	};

private:
	Engine& _engine;

	const ConstructorInfo _constructionInfo;

	std::vector<ShaderProgram> _programs;

	std::unordered_map<std::string, GLuint> _textures;
	std::unordered_map<std::string, std::tuple<GLuint, GLuint, GLuint, GLuint>> _meshes;
	std::unordered_map<std::string, GLuint> _shaders;
	std::unordered_map<std::string, uint32_t> _shadersToProgram;

	float _verticalFov = 0.f;
	float _zDepth = 0.f;
	glm::vec2 _size;

	glm::mat4 _projectionMatrix;

	Engine::Entity _camera;

	bool _rendering = false;

	void _reshape();

	bool _compileShader(GLuint type, GLuint* shader, const std::string & file);

public:
	Renderer(Engine& engine, const ConstructorInfo& constructionInfo = ConstructorInfo());

	void initiate(const std::vector<std::string>& args) override;
	//void update(double dt) override;
	void windowOpen(bool opened) override;
	void framebufferSize(glm::uvec2 size) override;

	void render() override;

	void textureLoaded(uint64_t id, const std::string& file, const TextureData* textureData) override;
	void meshLoaded(uint64_t id, const std::string& file, const MeshData* meshData) override;

	void reshape(const ShapeInfo& config);

	void loadProgram(const std::string& vertexFile, const std::string& fragmentFile, uint64_t id = 0, bool reload = false);

	void setCamera(uint64_t id);

	glm::mat4 viewMatrix();
};