#pragma once

#include "SystemInterface.hpp"

#include <string>
#include <unordered_map>

class AssetLoader : public SystemInterface {
	Engine& _engine;

	std::string _path;
	std::string _folder;

	std::unordered_map<std::string, uint8_t> _loaded;

public:
	AssetLoader(Engine& engine);

	void initiate(const std::vector<std::string>& args) override;

	void setFolder(const std::string& folder);

	void loadMesh(const std::string& file, uint64_t id = 0, bool reload = false);
	void loadTexture(const std::string& file, uint64_t id = 0, bool reload = false);
};