#include "Renderer.hpp"

#include <glad\glad.h>
#include <glm\gtc\matrix_transform.hpp>
#include <fstream>

#include "Transform.hpp"

void errorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string errorMessage(message, message + length);
	std::cerr << source << ',' << type << ',' << id << ',' << severity << std::endl << errorMessage << std::endl << std::endl;
}

void Renderer::_reshape(){
	if (_verticalFov && _size.x && _size.y && _zDepth)
		_projectionMatrix = glm::perspectiveFov(glm::radians(_verticalFov), _size.x, _size.y, 1.f, _zDepth);

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

	std::cerr << (char*)(&message[0]) << std::endl << std::endl;
	return false;
}

Renderer::Renderer(Engine& engine, const ConstructorInfo& constructionInfo) : _engine(engine), _constructionInfo(constructionInfo), _camera(engine){
	SYSFUNC_ENABLE(SystemInterface, initiate, 0);
	SYSFUNC_ENABLE(SystemInterface, update, 1);

	SYSFUNC_ENABLE(SystemInterface, framebufferSize, 0);
	SYSFUNC_ENABLE(SystemInterface, textureLoaded, 0);
	SYSFUNC_ENABLE(SystemInterface, meshLoaded, 0);
}

void Renderer::initiate(const std::vector<std::string>& args){
	_rendering = true;

	glDebugMessageCallback(errorCallback, nullptr);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DITHER);

	_reshape();
}

void Renderer::update(double dt){
	if (!_rendering)
		return;

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_engine.iterateEntities([&](Engine::Entity& entity) {
		if (!entity.has<Transform, Model>())
			return;

		const Transform& transform = *entity.get<Transform>();
		const Model& model = *entity.get<Model>();

		if (!model.arrayObject || !model.indexBuffer || !model.vertexBuffer || !model.program || !model.textureBuffer || !model.indexCount)
			return;

		const ShaderProgram& program = _programs[model.program - 1];

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
			modelMatrix = transform.matrix();

			if (program.viewUnifLoc != -1)
				glUniformMatrix4fv(program.modelUnifLoc, 1, GL_FALSE, &((glm::mat4)modelMatrix)[0][0]);
		}

		// model view matrix
		if (program.modelViewUnifLoc != -1)
			glUniformMatrix4fv(program.modelViewUnifLoc, 1, GL_FALSE, &((glm::mat4)(viewMatrix * modelMatrix))[0][0]);

		// texture
		if (program.textureUnifLoc != -1) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model.textureBuffer);

			glUniform1i(program.textureUnifLoc, 0);
		}

		// mesh
		glBindVertexArray(model.arrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, model.vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);

		glDrawElements(GL_TRIANGLES, model.indexCount, GL_UNSIGNED_INT, 0);

		// clean up
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
	});

	SYSFUNC_CALL(SystemInterface, rendered, _engine)();
}
/*
void Renderer::windowOpen(bool opened){
	_rendering = opened;

	if (!opened)
		return;

	glDebugMessageCallback(errorCallback, nullptr);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DITHER);

	_reshape();
}
*/

void Renderer::framebufferSize(glm::uvec2 size){
	_size.x = size.x;
	_size.y = size.y;

	if (_rendering)
		_reshape();
}

void Renderer::textureLoaded(uint64_t id, const std::string& file, const TextureData* textureData){
	GLuint textureBuffer;

	bool exists = false;

	if (_textures.find(file) != _textures.end()) {
		textureBuffer = _textures[file];
		exists = true;
	}

	if (textureData) {
		if (!exists)
			glGenTextures(1, &textureBuffer);
		
		glBindTexture(GL_TEXTURE_2D, textureBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureData->size.x, textureData->size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureData->colours[0]);
		
		//glGenerateMipmap(GL_TEXTURE_2D);
		//glGenerateTextureMipmap(textureBuffer);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		_textures[file] = textureBuffer;
	}

	if (_engine.validEntity(id)) {
		Model& model = *_engine.addComponent<Model>(id);
		model.textureBuffer = textureBuffer;
	}
}

#define SetAttribPointer(attribute, vertex, member) \
	glEnableVertexAttribArray(attribute); \
	glVertexAttribPointer(attribute, sizeof(vertex::member) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, member))

void Renderer::meshLoaded(uint64_t id, const std::string& file, const MeshData* meshData){
	GLuint arrayObject;
	GLuint indexBuffer;
	GLuint vertexBuffer;
	GLuint indexCount;

	bool exists = false;

	if (_meshes.find(file) != _meshes.end()) {
		arrayObject = std::get<0>(_meshes[file]);
		indexBuffer = std::get<1>(_meshes[file]);
		vertexBuffer = std::get<2>(_meshes[file]);
		indexCount = std::get<3>(_meshes[file]);
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
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		indexCount = meshData->indexes.size();
		_meshes[file] = { arrayObject, indexBuffer, vertexBuffer, indexCount };
	}

	if (_engine.validEntity(id)) {
		Model& model = *_engine.addComponent<Model>(id);
		model.arrayObject = arrayObject;
		model.indexBuffer = indexBuffer;
		model.vertexBuffer = vertexBuffer;
		model.indexCount = indexCount;
	}
}

void Renderer::setShape(const ShapeConfig& config){
	_verticalFov = config.verticalFov;
	_zDepth = config.zDepth;

	if (_rendering)
		_reshape();
}

void Renderer::loadProgram(const std::string& vertexFile, const std::string& fragmentFile, uint64_t id, bool reload) {
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;

	if (_shaders.find(vertexFile) != _shaders.end())
		vertexShader = _shaders[vertexFile];

	if (_shaders.find(fragmentFile) != _shaders.end())
		fragmentShader = _shaders[fragmentFile];

	bool newProgram = !vertexShader || !fragmentShader;
	
	if (!vertexShader || reload) {
		_compileShader(GL_VERTEX_SHADER, &vertexShader, vertexFile);
		_shaders[vertexFile] = vertexShader;
	}
	
	if (!fragmentShader || reload) {
		_compileShader(GL_FRAGMENT_SHADER, &fragmentShader, fragmentFile);
		_shaders[fragmentFile] = fragmentShader;
	}
	
	std::string programFiles = vertexFile + '/' + fragmentFile;
	uint32_t programIndex;
	
	if (_shadersToProgram.find(programFiles) != _shadersToProgram.end()) {
		programIndex = _shadersToProgram[programFiles];
	}
	else {
		programIndex = _programs.size();
		_programs.resize(_programs.size() + 1);
	
		_shadersToProgram[programFiles] = programIndex;
	}
	
	if (newProgram || reload) {
		ShaderProgram& program = _programs[programIndex];
	
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
	
			std::cerr << (char*)(&message[0]) << std::endl << std::endl;
			return;
		}
	
		program.modelUnifLoc = glGetUniformLocation(program.program, _constructionInfo.modelUnifName.c_str());
		program.viewUnifLoc = glGetUniformLocation(program.program, _constructionInfo.viewUnifName.c_str());
		program.projectionUnifLoc = glGetUniformLocation(program.program, _constructionInfo.projectionUnifName.c_str());
		program.modelViewUnifLoc = glGetUniformLocation(program.program, _constructionInfo.modelViewUnifName.c_str());
		program.textureUnifLoc = glGetUniformLocation(program.program, _constructionInfo.textureUnifName.c_str());
	}
	
	if (_engine.validEntity(id)) {
		Model& model = *_engine.addComponent<Model>(id);
		model.program = programIndex + 1;
	}
}

void Renderer::setCamera(uint64_t id){
	if (!_engine.validEntity(id))
		return;

	_camera.set(id);
}

glm::mat4 Renderer::viewMatrix(){
	glm::mat4 matrix;

	if (_camera.valid() && _camera.has<Transform>())
		matrix = glm::inverse(_camera.get<Transform>()->matrix());;

	return matrix;
}