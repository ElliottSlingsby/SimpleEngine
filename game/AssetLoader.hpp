#pragma once

#include "SystemInterface.hpp"

#include <string>
#include <unordered_map>

class AssetLoader : public SystemInterface {
	Engine& _engine;

	std::unordered_map<std::string, uint8_t> _loaded;

	//void _loadShader(const std::string& file, ShaderData::Type type, uint64_t id, bool reload);

public:
	AssetLoader(Engine& engine);

	void initiate(const std::vector<std::string>& args) override;

	void loadMesh(const std::string& file, uint64_t id = 0, bool reload = false);
	void loadTexture(const std::string& file, uint64_t id = 0, bool reload = false);
	//void loadShaders(const std::string& vertexFile, const std::string& fragmentFile, uint64_t id = 0, bool reload = false);
};