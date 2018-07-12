#pragma once

#include "SystemInterface.hpp"

#include <string>

class AssetLoader : public SystemInterface {
	Engine& _engine;

	const std::string _folder;
	std::string _path;

public:
	AssetLoader(Engine& engine, const std::string& folder);

	void initiate(int argc, char** argv) override;

	void loadMesh(const std::string& file, uint64_t id = 0);
	void loadTexture(const std::string& file, uint64_t id = 0);
	void loadFont(const std::string& file, const std::string& text, uint64_t id = 0);
};