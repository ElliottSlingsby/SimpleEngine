#pragma once

#include "SystemInterface.hpp"
#include "Window.hpp"

#include <vector>
#include <tuple>
#include <unordered_map>

struct Model {

};

struct Material {

};

class Renderer : public SystemInterface {
	Engine& _engine;
	std::unordered_map<std::string, GLuint> _shaders;

public:
	enum ShaderType {
		VertexShader,
		FragmentShader,
		GeometryShader
	};

	Renderer(Engine& engine);

	void initiate(int argc, char** argv) override;
	void update(double dt) override;
	void framebufferSize(int width, int height) override;

	uint32_t createProgram(const std::vector<std::tuple<ShaderType, std::string>> shaders);
};