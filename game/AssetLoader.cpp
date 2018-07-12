#include "AssetLoader.hpp"

AssetLoader::AssetLoader(Engine & engine, const std::string& folder) : _engine(engine), _folder(folder){
	SYSFUNC_ENABLE(SystemInterface, initiate, -1);
}

void AssetLoader::initiate(int argc, char ** argv){
	assert(argc);
	_path = upperPath(replace('\\', '/', argv[0])) + _folder + '/';
}

void AssetLoader::loadMesh(const std::string& file, uint64_t id){
	SYSFUNC_CALL(SystemInterface, loadedMesh, _engine)(file, id);
}

void AssetLoader::loadTexture(const std::string& file, uint64_t id){
	SYSFUNC_CALL(SystemInterface, loadedTexture, _engine)(file, id);
}

void AssetLoader::loadFont(const std::string& file, const std::string & text, uint64_t id){
	SYSFUNC_CALL(SystemInterface, loadedTexture, _engine)(file + '/' + text, id);
}