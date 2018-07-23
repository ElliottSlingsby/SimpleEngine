#include "AssetLoader.hpp"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <stb_image.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>

inline void fromAssimp(const aiVector3D& const from, glm::vec2* to) {
	to->x = from.x;
	to->y = from.y;
}

inline void fromAssimp(const aiVector3D& from, glm::vec3* to) {
	to->x = from.x;
	to->y = from.y;
	to->z = from.z;
}

inline void fromAssimp(const aiColor4D& const from, glm::vec4* to) {
	to->r = from.r;
	to->g = from.g;
	to->b = from.b;
	to->a = from.a;
}

AssetLoader::AssetLoader(Engine& engine) : _engine(engine){
	SYSFUNC_ENABLE(SystemInterface, initiate, -1);
}

void AssetLoader::initiate(const std::vector<std::string>& args){
	stbi_set_flip_vertically_on_load(true);
}

void AssetLoader::loadMesh(const std::string& file, uint64_t id, bool reload){
	if (_loaded.find(file) != _loaded.end()) {
		SYSFUNC_CALL(SystemInterface, meshLoaded, _engine)(id, file, nullptr);
		return;
	}
	
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(file,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_GenUVCoords
	);

	if (!scene || !scene->mNumMeshes)
		return;

	MeshData meshData;

	const aiMesh& mesh = *scene->mMeshes[0];

	meshData.indexes.resize(mesh.mNumFaces * 3);
	
	for (uint32_t x = 0; x < mesh.mNumFaces; x++) {
		const aiFace& face = mesh.mFaces[x];
		
		for (uint32_t y = 0; y < 3; y++)
			meshData.indexes[(x * 3) + y] = face.mIndices[y];
	}

	meshData.vertices.resize(mesh.mNumVertices);

	for (uint32_t i = 0; i < mesh.mNumVertices; i++) {
		MeshData::Vertex& vertex = meshData.vertices[i];

		fromAssimp(mesh.mVertices[i], &vertex.position);

		if (mesh.HasNormals())
			fromAssimp(mesh.mNormals[i], &vertex.normal);
		
		if (mesh.HasTextureCoords(0))
			fromAssimp(mesh.mTextureCoords[0][i], &vertex.texcoord);
		
		if (mesh.HasVertexColors(0))
			fromAssimp(mesh.mColors[0][i], &vertex.colour);

		if (mesh.HasTangentsAndBitangents()) {
			fromAssimp(mesh.mTangents[i], &vertex.tangent);
			fromAssimp(mesh.mBitangents[i], &vertex.bitangent);
		}
	}

	_loaded[file] = 1;

	SYSFUNC_CALL(SystemInterface, meshLoaded, _engine)(id, file, &meshData);
}

void AssetLoader::loadTexture(const std::string& file, uint64_t id, bool reload){
	if (_loaded.find(file) != _loaded.end()) {
		SYSFUNC_CALL(SystemInterface, textureLoaded, _engine)(id, file, nullptr);
		return;
	}

	int width, height, channels;
	uint8_t* data = stbi_load(file.c_str(), &width, &height, &channels, 4);

	if (!data)
		return;

	TextureData textureData;

	textureData.channels = glm::clamp(channels, 0, 4);
	textureData.size = { glm::clamp(width, 0, INT32_MAX), glm::clamp(height, 0, INT32_MAX) };
	
	textureData.colours.resize(textureData.size.x * textureData.size.y);
	memcpy(&textureData.colours[0], data, textureData.size.x * textureData.size.y * 4);

	stbi_image_free(data);

	_loaded[file] = 1;

	SYSFUNC_CALL(SystemInterface, textureLoaded, _engine)(id, file, &textureData);
}