#include "Renderer.hpp"

#include <glad\glad.h>
#include <glm\gtc\matrix_transform.hpp>
#include <fstream>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <stb_image.h>
#include <stb_rect_pack.h>
#include <stb_truetype.h>

#include "Transform.hpp"

//#define BufferSubData(attribute, offset, size, count, data) \
//	glEnableVertexAttribArray(attribute); \
//	glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(float), size * sizeof(float), (void*)data); \
//	glVertexAttribPointer(attribute, count, GL_FLOAT, GL_FALSE, 0, (void*)(offset * sizeof(float)))

inline void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string errorMessage(message, message + length);
	std::cerr << source << ',' << type << ',' << id << ',' << severity << std::endl << errorMessage << std::endl << std::endl;
}

void Renderer::_reshape(){
	if (_shapeInfo.verticalFov && _size.x && _size.y && _shapeInfo.zDepth)
		_projectionMatrix = glm::perspectiveFov(glm::radians(_shapeInfo.verticalFov), _size.x, _size.y, 1.f, _shapeInfo.zDepth);

	glViewport(0, 0, _size.x, _size.y);
}

bool Renderer::_compileShader(GLuint type, GLuint* shader, const std::string & file){
	if (*shader == 0)
		*shader = glCreateShader(type);

	std::ifstream stream;

	stream.open(file, std::ios::in);

	if (!stream.is_open())
		return false;

	const std::string source = std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

	stream.close();

	const GLchar* sourcePtr = (const GLchar*)(source.c_str());

	glShaderSource(*shader, 1, &sourcePtr, 0);
	glCompileShader(*shader);

	GLint compiled;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_TRUE)
		return true;

	GLint length = 0;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &length);

	std::vector<GLchar> message(length);
	glGetShaderInfoLog(*shader, length, &length, &message[0]);

	glDeleteShader(*shader);
	*shader = 0;

	std::cerr << (char*)(&message[0]) << std::endl << std::endl;
	return false;
}

void Renderer::_addModel(uint64_t id, uint32_t mesh, uint32_t texture, GLuint program) {
	Model& model = *_engine.addComponent<Model>(id);

	if (mesh)
		model.meshContextId = mesh;

	if (program)
		model.programContextId = program;
	else if (!model.programContextId && _defaultProgram)
		model.programContextId = _defaultProgram;

	if (texture)
		model.textureBufferId = texture;
	else if (!model.textureBufferId && _defaultTexture)
		model.textureBufferId = _defaultTexture;
}

void Renderer::_bufferMesh(MeshContext* meshContext, const aiMesh& mesh){
	if (!meshContext->indexCount) {
		assert(!meshContext->arrayObject && !meshContext->indexBuffer && !meshContext->vertexBuffer); // sanity

		glGenVertexArrays(1, &meshContext->arrayObject);
		glGenBuffers(1, &meshContext->vertexBuffer);
		glGenBuffers(1, &meshContext->indexBuffer);
	}

	// bind buffers
	glBindVertexArray(meshContext->arrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshContext->indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshContext->vertexBuffer);

	// buffer index data
	meshContext->indexCount = mesh.mNumFaces * 3;

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshContext->indexCount * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

	for (uint32_t i = 0; i < mesh.mNumFaces; i++)
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i * sizeof(uint32_t) * 3, sizeof(uint32_t) * 3, mesh.mFaces[i].mIndices);

	// buffer vertex data
	size_t positionsSize = 3 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasPositions();
	size_t textureCoordsSize = 2 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasTextureCoords(0);
	size_t normalSize = 3 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasNormals();

	glBufferData(GL_ARRAY_BUFFER, positionsSize + normalSize + textureCoordsSize, 0, GL_STATIC_DRAW);

	if (positionsSize) {
		glEnableVertexAttribArray(_constructionInfo.positionAttrLoc);
		glVertexAttribPointer(_constructionInfo.positionAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, mesh.mVertices);
	}

	if (normalSize) {
		glEnableVertexAttribArray(_constructionInfo.normalAttrLoc);
		glVertexAttribPointer(_constructionInfo.normalAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionsSize));
		glBufferSubData(GL_ARRAY_BUFFER, positionsSize, normalSize, mesh.mNormals);
	}

	if (textureCoordsSize) {
		glEnableVertexAttribArray(_constructionInfo.texcoordAttrLoc);
		glVertexAttribPointer(_constructionInfo.texcoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(positionsSize + normalSize));

		for (uint32_t i = 0; i < mesh.mNumVertices; i++)
			glBufferSubData(GL_ARRAY_BUFFER, positionsSize + normalSize + (i * 2 * sizeof(float)), 2 * sizeof(float), &mesh.mTextureCoords[0][i]);
	}
}

Renderer::Renderer(Engine& engine, const ConstructorInfo& constructionInfo) : _engine(engine), _constructionInfo(constructionInfo), _camera(engine){
	SYSFUNC_ENABLE(SystemInterface, initiate, 0);
	SYSFUNC_ENABLE(SystemInterface, update, 1);

	SYSFUNC_ENABLE(SystemInterface, framebufferSize, 0);
	SYSFUNC_ENABLE(SystemInterface, windowOpen, 0);
}

void Renderer::initiate(const std::vector<std::string>& args){
	glDebugMessageCallback(errorCallback, nullptr);

	stbi_set_flip_vertically_on_load(true);
}

void Renderer::windowOpen(bool opened){
	_rendering = opened;

	if (!opened)
		return;

	//glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DITHER);

	
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	_reshape();
}

void Renderer::framebufferSize(glm::uvec2 size){
	_size.x = size.x;
	_size.y = size.y;

	if (_rendering)
		_reshape();
}

void Renderer::update(double dt){
	if (!_rendering)
		return;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_engine.iterateEntities([&](Engine::Entity& entity) {
		if (!entity.has<Transform, Model>())
			return;

		const Transform& transform = *entity.get<Transform>();
		Model& model = *entity.get<Model>();

		if (!model.meshContextId)
			return;

		// search upwards and copy over texturebufferid and programcontextid

		if (!model.programContextId || !model.textureBufferId)
			return;

		const ProgramContext& program = _programContexts[model.programContextId - 1];

		glUseProgram(program.program);

		// projection matrix
		if (program.projectionUnifLoc != -1)
			glUniformMatrix4fv(program.projectionUnifLoc, 1, GL_FALSE, &_projectionMatrix[0][0]);

		// view matrix
		glm::dmat4 viewMatrix = Renderer::viewMatrix();

		if (program.viewUnifLoc != -1)
			glUniformMatrix4fv(program.viewUnifLoc, 1, GL_FALSE, &((glm::mat4)viewMatrix)[0][0]);

		// model matrix
		glm::dmat4 modelMatrix;

		if (program.modelViewUnifLoc != -1 || program.viewUnifLoc != -1) {
			modelMatrix = transform.globalMatrix(_engine);

			if (program.viewUnifLoc != -1)
				glUniformMatrix4fv(program.modelUnifLoc, 1, GL_FALSE, &((glm::mat4)modelMatrix)[0][0]);
		}

		// model view matrix
		if (program.modelViewUnifLoc != -1)
			glUniformMatrix4fv(program.modelViewUnifLoc, 1, GL_FALSE, &((glm::mat4)(viewMatrix * modelMatrix))[0][0]);

		// texture
		if (program.textureUnifLoc != -1) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model.textureBufferId);

			glUniform1i(program.textureUnifLoc, 0);
		}

		// mesh
		MeshContext& meshContext = _meshContexts[model.meshContextId - 1];

		glBindVertexArray(meshContext.arrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, meshContext.vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshContext.indexBuffer);

		glDrawElements(GL_TRIANGLES, meshContext.indexCount, GL_UNSIGNED_INT, 0);
	});
}

void Renderer::reshape(const ShapeInfo& config){
	_shapeInfo.verticalFov = config.verticalFov;
	_shapeInfo.zDepth = config.zDepth;

	if (_rendering)
		_reshape();
}

uint32_t Renderer::loadProgram(const std::string& vertexFile, const std::string& fragmentFile, uint64_t id, bool reload) {
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;

	if (_shaderFiles.find(vertexFile) != _shaderFiles.end())
		vertexShader = _shaderFiles[vertexFile];

	if (_shaderFiles.find(fragmentFile) != _shaderFiles.end())
		fragmentShader = _shaderFiles[fragmentFile];

	bool newProgram = !vertexShader || !fragmentShader;
	bool success = true;
	
	if (!vertexShader || reload) {
		success &= _compileShader(GL_VERTEX_SHADER, &vertexShader, vertexFile);
		_shaderFiles[vertexFile] = vertexShader;
	}
	
	if (!fragmentShader || reload) {
		success &= _compileShader(GL_FRAGMENT_SHADER, &fragmentShader, fragmentFile);
		_shaderFiles[fragmentFile] = fragmentShader;
	}
	
	if (!success)
		return 0;

	std::string programFiles = vertexFile + '/' + fragmentFile;
	uint32_t programIndex;
	
	if (_programFiles.find(programFiles) != _programFiles.end()) {
		programIndex = _programFiles[programFiles];
	}
	else {
		programIndex = _programContexts.size();
		_programContexts.resize(_programContexts.size() + 1);
	
		_programFiles[programFiles] = programIndex;
	}
	
	if (newProgram || reload) {
		ProgramContext& program = _programContexts[programIndex];
	
		if (!program.program) {
			program.program = glCreateProgram();
	
			glAttachShader(program.program, vertexShader);
			glAttachShader(program.program, fragmentShader);
	
			glLinkProgram(program.program);
		}
	
		GLint success;
		glGetProgramiv(program.program, GL_LINK_STATUS, &success);
	
		if (!success) {
			GLint length = 0;
			glGetProgramiv(program.program, GL_INFO_LOG_LENGTH, &length);
	
			std::vector<GLchar> message(length);
			glGetProgramInfoLog(program.program, length, &length, &message[0]);
	
			glDeleteProgram(program.program);
			program.program = 0;

			std::cerr << (char*)(&message[0]) << std::endl << std::endl;
			return 0;
		}
	
		program.modelUnifLoc = glGetUniformLocation(program.program, _constructionInfo.modelUnifName.c_str());
		program.viewUnifLoc = glGetUniformLocation(program.program, _constructionInfo.viewUnifName.c_str());
		program.projectionUnifLoc = glGetUniformLocation(program.program, _constructionInfo.projectionUnifName.c_str());
		program.modelViewUnifLoc = glGetUniformLocation(program.program, _constructionInfo.modelViewUnifName.c_str());
		program.textureUnifLoc = glGetUniformLocation(program.program, _constructionInfo.textureUnifName.c_str());
	}
	
	if (_engine.validEntity(id)) {
		Model& model = *_engine.addComponent<Model>(id);
		model.programContextId = programIndex + 1;
	}

	return programIndex + 1;
}

GLuint Renderer::loadTexture(const std::string & textureFile, uint64_t id, bool reload){
	// check if texture already loaded
	auto iter = _textureFiles.find(textureFile);

	if (!reload && iter != _textureFiles.end()) {
		if (id)
			_addModel(id, 0, iter->second);

		return iter->second;
	}
	
	// if not, or reloading, load from file
	int width, height, channels;
	uint8_t* data = stbi_load(textureFile.c_str(), &width, &height, &channels, 4);

	if (!data)
		return 0;

	// buffer data to opengl
	GLuint textureBuffer;

	if (iter != _textureFiles.end())
		textureBuffer = iter->second;
	else
		glGenTextures(1, &textureBuffer);

	glBindTexture(GL_TEXTURE_2D, textureBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)data);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(data);

	if (id)
		_addModel(id, 0, textureBuffer);

	return textureBuffer;
}

uint32_t Renderer::loadMesh(const std::string& meshFile, uint64_t id, bool reload){
	// if not reloading, and mesh already loaded, set model to existing or ignore if no id
	auto iter = _meshFiles.find(meshFile);

	if (!reload && iter != _meshFiles.end()) {
		uint32_t index = iter->second + 1;
	
		if (id)
			_addModel(id, index);
	
		return index;
	}

	// mesh not found or reloading, therefore load mesh from file
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(meshFile,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType //|
		//aiProcess_GenUVCoords |
		//aiProcess_FindInstances
	);

	if (!scene || !scene->mNumMeshes)
		return 0;

	// upload mesh data to opengl
	const aiMesh& mesh = *scene->mMeshes[0];




	MeshContext* meshContext;
	uint32_t meshIndex;

	if (iter == _meshFiles.end()) {
		meshIndex = _meshContexts.size();
		_meshContexts.resize(_meshContexts.size() + 1);
		meshContext = &*_meshContexts.rbegin();
	}
	else {
		meshIndex = iter->second;
		meshContext = &_meshContexts[iter->second];
	}

	_bufferMesh(meshContext, mesh);
	
	/*
	// create and bind mesh context if non existant, or get existing one even if reloading
	MeshContext* meshContext;
	uint32_t meshIndex;

	if (iter == _meshFiles.end()) {
		meshIndex = _meshContexts.size();
		_meshContexts.resize(_meshContexts.size() + 1);
		meshContext = &*_meshContexts.rbegin();

		glGenVertexArrays(1, &meshContext->arrayObject);
		glGenBuffers(1, &meshContext->vertexBuffer);
		glGenBuffers(1, &meshContext->indexBuffer);
	}
	else {
		meshIndex = iter->second;
		meshContext = &_meshContexts[iter->second];
	}

	// bind buffers
	glBindVertexArray(meshContext->arrayObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshContext->indexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, meshContext->vertexBuffer);

	// buffer index data
	meshContext->indexCount = mesh.mNumFaces * 3;

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshContext->indexCount * sizeof(uint32_t), nullptr, GL_STATIC_DRAW);

	for (uint32_t i = 0; i < mesh.mNumFaces; i++)
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i * sizeof(uint32_t) * 3, sizeof(uint32_t) * 3, mesh.mFaces[i].mIndices);

	// buffer vertex data
	size_t positionsSize = 3 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasPositions();
	size_t textureCoordsSize = 2 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasTextureCoords(0);
	size_t normalSize = 3 * mesh.mNumVertices * sizeof(float)* (uint32_t)mesh.HasNormals();

	glBufferData(GL_ARRAY_BUFFER, positionsSize + normalSize + textureCoordsSize, 0, GL_STATIC_DRAW);

	if (positionsSize) {
		glEnableVertexAttribArray(_constructionInfo.positionAttrLoc);
		glVertexAttribPointer(_constructionInfo.positionAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
		glBufferSubData(GL_ARRAY_BUFFER, 0, positionsSize, mesh.mVertices);
	}
	
	if (normalSize) {
		glEnableVertexAttribArray(_constructionInfo.normalAttrLoc);
		glVertexAttribPointer(_constructionInfo.normalAttrLoc, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionsSize));
		glBufferSubData(GL_ARRAY_BUFFER, positionsSize, normalSize, mesh.mNormals);
	}

	if (textureCoordsSize) {
		glEnableVertexAttribArray(_constructionInfo.texcoordAttrLoc);
		glVertexAttribPointer(_constructionInfo.texcoordAttrLoc, 2, GL_FLOAT, GL_FALSE, 0, (void*)(positionsSize + normalSize));

		for (uint32_t i = 0; i < mesh.mNumVertices; i++)
			glBufferSubData(GL_ARRAY_BUFFER, positionsSize + normalSize + (i * 2 * sizeof(float)), 2 * sizeof(float), &mesh.mTextureCoords[0][i]);
	}
	*/

	if (id)
		_addModel(id, meshIndex + 1);

	return meshIndex + 1;
}

void Renderer::defaultProgram(const std::string& vertexFile, const std::string& fragmentFile){
	_defaultProgram = loadProgram(vertexFile, fragmentFile);
}

void Renderer::defaultTexture(const std::string & textureFile){
	_defaultTexture = loadTexture(textureFile);
}

void Renderer::setCamera(uint64_t id){
	_camera.set(id);
}

glm::mat4 Renderer::viewMatrix() const{
	glm::mat4 matrix;

	if (_camera.valid() && _camera.has<Transform>())
		matrix = glm::inverse(_camera.get<Transform>()->globalMatrix(_engine));

	return matrix;
}

/*
void Renderer::textureLoaded(uint64_t id, const std::string& file, const TextureData* textureData){
	GLuint textureBuffer;

	bool exists = false;

	if (_textureFiles.find(file) != _textureFiles.end()) {
		textureBuffer = _textureFiles[file];
		exists = true;
	}

	if (textureData) {
		if (!exists)
			glGenTextures(1, &textureBuffer);
		
		glBindTexture(GL_TEXTURE_2D, textureBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureData->size.x, textureData->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureData->colours[0]);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		_textureFiles[file] = textureBuffer;
	}

	if (_engine.validEntity(id)) {
		Model& model = *_engine.addComponent<Model>(id);
		model.textureBuffer = textureBuffer;
	}
}

void Renderer::meshLoaded(uint64_t id, const std::string& file, const MeshData* meshData){
	GLuint arrayObject;
	GLuint indexBuffer;
	GLuint vertexBuffer;
	GLuint indexCount;

	bool exists = false;

	if (_meshFiles.find(file) != _meshFiles.end()) {
		arrayObject = std::get<0>(_meshFiles[file]);
		indexBuffer = std::get<1>(_meshFiles[file]);
		vertexBuffer = std::get<2>(_meshFiles[file]);
		indexCount = std::get<3>(_meshFiles[file]);
		exists = true;
	}

	if (meshData){
		if (!exists) {
			glGenVertexArrays(1, &arrayObject);
			glGenBuffers(1, &vertexBuffer);
			glGenBuffers(1, &indexBuffer);
		}
		
		glBindVertexArray(arrayObject);
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, meshData->vertices.size() * sizeof(MeshData::Vertex), &meshData->vertices[0], GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->indexes.size() * sizeof(uint32_t), &meshData->indexes[0], GL_STATIC_DRAW);
		
		SetAttribPointer(_constructionInfo.positionAttrLoc, MeshData::Vertex, position);
		SetAttribPointer(_constructionInfo.normalAttrLoc, MeshData::Vertex, normal);
		SetAttribPointer(_constructionInfo.texcoordAttrLoc, MeshData::Vertex, texcoord);
		SetAttribPointer(_constructionInfo.colourAttrLoc, MeshData::Vertex, colour);
		SetAttribPointer(_constructionInfo.tangentAttrLoc, MeshData::Vertex, tangent);
		SetAttribPointer(_constructionInfo.bitangentAttrLoc, MeshData::Vertex, bitangent);

		indexCount = meshData->indexes.size();
		_meshFiles[file] = { arrayObject, indexBuffer, vertexBuffer, indexCount };
	}

	if (_engine.validEntity(id)) {
		Model& model = *_engine.addComponent<Model>(id);
		model.arrayObject = arrayObject;
		model.indexBuffer = indexBuffer;
		model.vertexBuffer = vertexBuffer;
		model.indexCount = indexCount;

		//if (_defaultVertexFile != "" && _defaultFragmentFile != "")
		//	loadProgram(_defaultVertexFile, _defaultFragmentFile, id);
	}
}
*/

/*
//glBufferData(GL_ARRAY_BUFFER, meshData->vertices.size() * sizeof(MeshData::Vertex), &meshData->vertices[0], GL_STATIC_DRAW);
//glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->indexes.size() * sizeof(uint32_t), &meshData->indexes[0], GL_STATIC_DRAW);

//glEnableVertexAttribArray(attribute);
//glVertexAttribPointer(attribute, sizeof(vertex::member) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, member));


glGenVertexArrays(1, &arrayObject);
glGenBuffers(1, &vertexBuffer);
glGenBuffers(1, &indexBuffer);

glBindVertexArray(arrayObject);

glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
glBufferData(GL_ARRAY_BUFFER, meshData->vertices.size() * sizeof(MeshData::Vertex), &meshData->vertices[0], GL_STATIC_DRAW);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData->indexes.size() * sizeof(uint32_t), &meshData->indexes[0], GL_STATIC_DRAW);

//glEnableVertexAttribArray(attribute);
//glVertexAttribPointer(attribute, sizeof(vertex::member) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, member));

SetAttribPointer(_constructionInfo.positionAttrLoc, MeshData::Vertex, position);
SetAttribPointer(_constructionInfo.normalAttrLoc, MeshData::Vertex, normal);
SetAttribPointer(_constructionInfo.texcoordAttrLoc, MeshData::Vertex, texcoord);
SetAttribPointer(_constructionInfo.colourAttrLoc, MeshData::Vertex, colour);
SetAttribPointer(_constructionInfo.tangentAttrLoc, MeshData::Vertex, tangent);
SetAttribPointer(_constructionInfo.bitangentAttrLoc, MeshData::Vertex, bitangent);
*/