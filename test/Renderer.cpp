#include "Renderer.hpp"

#include "Transform.hpp"
#include "Model.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Attributes {
	glm::tvec3<GLfloat> vertex;
	glm::tvec3<GLfloat> normal;
	glm::tvec2<GLfloat> texcoord;
};

int verboseCheckError() {
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return 0;

	std::cerr << error << std::endl;

	return 1;
}

#define glCheckError() assert(verboseCheckError() == 0)

void errorCallback(int error, const char* description) {
	std::cerr << "GLFW Error - " << error << " - " << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	Renderer& renderer = *(Renderer*)glfwGetWindowUserPointer(window);

	renderer._engine.events.dispatch(Events::Keypress, key, scancode, action, mods);
}

void cursorCallback(GLFWwindow* window, double x, double y){
	Renderer& renderer = *(Renderer*)glfwGetWindowUserPointer(window);
	
	renderer._engine.events.dispatch(Events::Cursor, x, y);
}

void windowSizeCallback(GLFWwindow* window, int height, int width) {
	Renderer& renderer = *(Renderer*)glfwGetWindowUserPointer(window);

	renderer._reshape(height, width);
}

bool compileShader(GLuint type, GLuint* shader, const std::string& src) {
	*shader = glCreateShader(type);

	const GLchar* srcPtr = static_cast<const GLchar*>(src.c_str());
	glShaderSource(*shader, 1, &srcPtr, 0);

	glCompileShader(*shader);

	GLint compiled;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_TRUE)
		return true;

	GLint length = 0;
	glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &length);

	std::vector<GLchar> message(length);
	glGetShaderInfoLog(*shader, length, &length, &message[0]);

	std::cerr << "GLSL error - " << static_cast<char*>(&message[0]) << std::endl;
	return false;
}

bool createProgram(GLuint* program, GLuint* vertexShader, GLuint* fragmentShader, const std::string& vertexSrc, const std::string& fragmentSrc) {
	// create shaders
	bool failed = false;

	if (!compileShader(GL_VERTEX_SHADER, vertexShader, vertexSrc))
		failed = true;

	if (!compileShader(GL_FRAGMENT_SHADER, fragmentShader, fragmentSrc))
		failed = true;

	if (failed) {
		glDeleteShader(*vertexShader);
		glDeleteShader(*fragmentShader);

		return false;
	}

	glCheckError();

	*program = glCreateProgram();
	glAttachShader(*program, *vertexShader);
	glAttachShader(*program, *fragmentShader);

	glLinkProgram(*program);

	// link the program
	GLint success;
	glGetProgramiv(*program, GL_LINK_STATUS, &success);

	if (!success) {
		GLint length = 0;
		glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> message(length);
		glGetProgramInfoLog(*program, length, &length, &message[0]);

		std::cerr << "GLSL error - " << static_cast<char*>(&message[0]) << std::endl;

		glDeleteShader(*vertexShader);
		glDeleteShader(*fragmentShader);
		glDeleteProgram(*program);

		return false;
	}

	return true;
}

void Renderer::_reshape(int height, int width) {
	_windowSize = { height, width };
	_projectionMatrix = glm::perspectiveFov(glm::radians(_fov), static_cast<float>(height), static_cast<float>(width), 1.f, 10000.f);

	glViewport(0, 0, height, width);
}

inline void applyTransform(Transform& transform, const aiMatrix4x4& aiMatrix) {
	glm::mat4 matrix(
		aiMatrix.a1, aiMatrix.a2, aiMatrix.a3, aiMatrix.a4,
		aiMatrix.b1, aiMatrix.b2, aiMatrix.b3, aiMatrix.b4,
		aiMatrix.c1, aiMatrix.c2, aiMatrix.c3, aiMatrix.c4,
		aiMatrix.d1, aiMatrix.d2, aiMatrix.d3, aiMatrix.d4
	);

	transform.position.x = aiMatrix.a4;
	transform.position.y = aiMatrix.b4;
	transform.position.z = aiMatrix.c4;

	transform.rotation = glm::quat_cast(matrix);
}

void Renderer::_extract(uint64_t parent, const aiScene* scene, const aiNode * node){
	_engine.entities.add<Transform>(parent);
	Transform& parentTransform = *_engine.entities.get<Transform>(parent);

	if (!strcmp(node->mName.C_Str(), "RootNode"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_Translation"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_RotationPivot"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_RotationOffset"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_PreRotation"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_PostRotation"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_ScalingPivot"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_ScalingOffset"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_Translation"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_Scaling"))
		applyTransform(parentTransform, node->mTransformation);
	else if (strcmpSuffix(node->mName.C_Str(), "$AssimpFbx$_Rotation"))
		applyTransform(parentTransform, node->mTransformation);

	uint64_t entity = parent;

	if (node->mNumMeshes) {
		entity = _engine.entities.create();
		_engine.entities.add<Transform>(entity);

		Transform& transform = *_engine.entities.get<Transform>(entity);
		transform.setParent(_engine.entities.get<Transform>(parent));

		applyTransform(transform, node->mTransformation);

		addShader(&entity, "vertexShader.glsl", "fragmentShader.glsl"); // remove me
		addMesh(&entity, "axis.obj"); // remove me
		addTexture(&entity, "rgb.png"); // remove me

		_engine.entities.add<Model>(entity);
		Model& model = *_engine.entities.get<Model>(entity);

		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			// buffer mesh data
		}
	}

	for (uint32_t i = 0; i < node->mNumChildren; i++)
		_extract(entity, scene, node->mChildren[i]);
}

Renderer::Renderer(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Load, &Renderer::load);
	_engine.events.subscribe(this, Events::Update, &Renderer::update);
}

Renderer::~Renderer() {
	if (_window)
		glfwDestroyWindow(_window);

	glfwTerminate();
}

void Renderer::load(int argc, char** argv) {
	// setup data stuff
	_path = upperPath(replace('\\', '/', argv[0])) + DATA_FOLDER + '/';
	_windowSize = { DEFUALT_WIDTH, DEFUALT_HEIGHT };
	_fov = DEFUALT_FOV;

	stbi_set_flip_vertically_on_load(true);

	// setup GLFW
	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		_engine.events.unsubscribe(this);
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	glfwWindowHint(GLFW_REFRESH_RATE, 1);

	_window = glfwCreateWindow(_windowSize.x, _windowSize.y, DEFAULT_TITLE, nullptr, nullptr);

	if (!_window) {
		std::cerr << "GLFW error - " << "cannot create window" << std::endl;
		_engine.events.unsubscribe(this);
		return;
	}

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	//glfwSwapInterval(1); // v-sync

	glfwSetWindowUserPointer(_window, this);

	glfwSetKeyCallback(_window, keyCallback);
	glfwSetWindowSizeCallback(_window, windowSizeCallback);

	lockCursor(true);
	glfwSetCursorPosCallback(_window, cursorCallback);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	_reshape(_windowSize.x, _windowSize.y);

	glCheckError();
}

void Renderer::update(double dt) {
	glfwPollEvents();

	if (glfwWindowShouldClose(_window)) {
		_engine.running = false;
		return;
	}

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_engine.entities.iterate<Transform, Model>([&](uint64_t id, Transform& transform, Model& model) {
		if (!model.hasProgram || (!model.texture && !model.arrayObject))
			return;

		Program& program = _programs[model.program];

		// setup program program
		glUseProgram(program.program);

		if (program.uniformTexture != -1)
			glUniform1i(program.uniformTexture, 0);

		// projection matrix
		if (program.uniformProjection != -1)
			glUniformMatrix4fv(program.uniformProjection, 1, GL_FALSE, &(_projectionMatrix)[0][0]);

		// view matrix
		glm::dmat4 viewMatrix;

		if ((program.uniformModelView != -1 || program.uniformView != -1) && _camera) {
			Transform& cameraTransform = *_engine.entities.get<Transform>(_camera);

			//viewMatrix = glm::scale(viewMatrix, static_cast<glm::dvec3>(cameraTransform.scale));
			viewMatrix *= glm::transpose(glm::mat4_cast(static_cast<glm::dquat>(cameraTransform.rotation)));
			viewMatrix = glm::translate(viewMatrix, -cameraTransform.position);

			if (program.uniformView != -1)
				glUniformMatrix4fv(program.uniformView, 1, GL_FALSE, &(static_cast<glm::mat4>(viewMatrix))[0][0]);
		}

		// model matrix
		glm::dmat4 modelMatrix;

		if (program.uniformModelView != -1 || program.uniformView != -1) {
			modelMatrix = glm::translate(modelMatrix, transform.worldPosition());
			modelMatrix *= glm::mat4_cast(static_cast<glm::dquat>(transform.worldRotation()));
			modelMatrix = glm::scale(modelMatrix, static_cast<glm::dvec3>(transform.worldScale()));

			if (program.uniformView != -1)
				glUniformMatrix4fv(program.uniformModel, 1, GL_FALSE, &(static_cast<glm::mat4>(modelMatrix))[0][0]);
		}

		// model view matrix
		if (program.uniformModelView != -1) 
			glUniformMatrix4fv(program.uniformModelView, 1, GL_FALSE, &(static_cast<glm::mat4>(viewMatrix * modelMatrix))[0][0]);

		// set texture
		if (program.uniformTexture != -1 && model.texture) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model.texture);

			if (model.linearTexture) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}

		// draw buffer
		if (model.arrayObject && model.attribBuffer && model.indexCount) {
			glBindVertexArray(model.arrayObject);
			glBindBuffer(GL_ARRAY_BUFFER, model.attribBuffer);

			glDrawArrays(GL_TRIANGLES, 0, model.indexCount);
		}

		// clean up
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glCheckError();
	});

	glUseProgram(0);
	glCheckError();

	glfwSwapBuffers(_window);
}

bool Renderer::addScene(uint64_t * id, const std::string & sceneFile) {
	if (!_engine.entities.valid(*id))
		return false;

	_engine.entities.add<Model>(*id);
	Model& model = *_engine.entities.get<Model>(*id);

	// assimp goes here
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(_path + sceneFile,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		//aiProcess_SortByPType |		// do i want this?
		aiProcess_RemoveComponent
	);

	if (!scene) {
		std::cerr << importer.GetErrorString() << std::endl;
		return false;
	}

	if (!scene->mNumMeshes)
		return false;

	_extract(*id, scene, scene->mRootNode);

	return true;
}

bool Renderer::addMesh(uint64_t* id, const std::string& meshFile) {
	if (!_engine.entities.valid(*id))
		return false;

	_engine.entities.add<Model>(*id);
	Model& model = *_engine.entities.get<Model>(*id);

	if (_meshes.find(meshFile) != _meshes.end()) {
		model.arrayObject = _meshes[meshFile].arrayObject;
		model.attribBuffer = _meshes[meshFile].attribBuffer;
		model.indexCount = _meshes[meshFile].indexCount;

		return true;
	}

	// load model data
	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string error;
	tinyobj::LoadObj(&attributes, &shapes, &materials, &error, (_path + meshFile).c_str());

	if (!error.empty()) {
		std::cerr << "cannot load mesh - " << _path + meshFile << std::endl;
		return false;
	}

	if (!attributes.vertices.size()) {
		std::cerr << "problem reading vertex data - " << _path + meshFile << std::endl;
		return false;
	}

	for (const tinyobj::shape_t& shape : shapes)
		model.indexCount += static_cast<GLsizei>(shape.mesh.indices.size());

	// create buffers
	glGenVertexArrays(1, &model.arrayObject);
	glBindVertexArray(model.arrayObject);

	glGenBuffers(1, &model.attribBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model.attribBuffer);
	glBufferData(GL_ARRAY_BUFFER, model.indexCount * sizeof(Attributes), nullptr, GL_STATIC_DRAW);

	// buffer data
	Attributes* attributeMap = static_cast<Attributes*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

	glCheckError();

	size_t depth = 0;

	for (const tinyobj::shape_t& shape : shapes) {
		for (size_t i = 0; i < shape.mesh.indices.size(); i++) {
			const tinyobj::index_t& index = shape.mesh.indices[i];
		
			memcpy(&attributeMap[depth + i].vertex, &attributes.vertices[(index.vertex_index * 3)], sizeof(Attributes::vertex));

			if (attributes.normals.size())
				memcpy(&attributeMap[depth + i].normal, &attributes.normals[(index.normal_index * 3)], sizeof(Attributes::normal));

			if (attributes.texcoords.size())
				memcpy(&attributeMap[depth + i].texcoord, &attributes.texcoords[(index.texcoord_index * 2)], sizeof(Attributes::texcoord));
		}

		depth += shape.mesh.indices.size();
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glCheckError();

	// assign attribute pointers
	glEnableVertexAttribArray(VERTEX_ATTRIBUTE);
	glVertexAttribPointer(VERTEX_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)(0));

	glEnableVertexAttribArray(NORMAL_ATTRIBUTE);
	glVertexAttribPointer(NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)(sizeof(Attributes::vertex)));

	glEnableVertexAttribArray(TEXCOORD_ATTRIBUTE);
	glVertexAttribPointer(TEXCOORD_ATTRIBUTE, 2, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)(sizeof(Attributes::vertex) + sizeof(Attributes::normal)));

	glCheckError();

	// clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glCheckError();

	_meshes[meshFile].arrayObject = model.arrayObject;
	_meshes[meshFile].attribBuffer = model.attribBuffer;
	 _meshes[meshFile].indexCount = model.indexCount;

	return true;
}

bool Renderer::addTexture(uint64_t* id, const std::string& textureFile) {
	if (!_engine.entities.valid(*id))
		return false;

	_engine.entities.add<Model>(*id);
	Model& model = *_engine.entities.get<Model>(*id);

	if (_textures.find(textureFile) != _textures.end()) {
		model.texture = _textures[textureFile];

		return true;
	}

	// load data
	int x, y, n;
	uint8_t* data = stbi_load((_path + textureFile).c_str(), &x, &y, &n, 4);

	if (!data) {
		std::cerr << "cannot load image - " << _path + textureFile << std::endl;
		return false;
	}

	// buffer data
	glGenTextures(1, &model.texture);
	glBindTexture(GL_TEXTURE_2D, model.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);
	glGenerateTextureMipmap(model.texture);

	glCheckError();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	glCheckError();

	stbi_image_free(data);

	_textures[textureFile] = model.texture;

	return true;
}

bool Renderer::addShader(uint64_t* id, const std::string& vertexShader, const std::string& fragmentShader) {
	if (!_engine.entities.valid(*id))
		return false;

	_engine.entities.add<Model>(*id);
	Model& model = *_engine.entities.get<Model>(*id);

	if (_shaders.find(vertexShader + fragmentShader) != _shaders.end()) {
		model.hasProgram = true;
		model.program = _shaders[vertexShader + fragmentShader];

		return true;
	}

	Program program;

	std::string vertexSrc = readFile(_path + vertexShader);
	std::string fragmentSrc = readFile(_path + fragmentShader);

	if ((vertexSrc == "" || fragmentSrc == "") || !createProgram(&program.program, &program.vertexShader, &program.fragmentShader, vertexSrc, fragmentSrc)) {
		std::cerr << "cannot create program program" << std::endl;

		if (vertexSrc == "")
			std::cerr << _path << vertexShader << std::endl;

		if (fragmentSrc == "")
			std::cerr << _path << fragmentShader << std::endl;

		return false;
	}

	// get uniform locations
	program.uniformModel = glGetUniformLocation(program.program, MODEL_UNIFORM);
	program.uniformView = glGetUniformLocation(program.program, VIEW_UNIFORM);
	program.uniformProjection = glGetUniformLocation(program.program, PROJECTION_UNIFORM);
	program.uniformModelView = glGetUniformLocation(program.program, MODELVIEW_UNIFORM);
	program.uniformTexture = glGetUniformLocation(program.program, TEXTURE_UNIFORM);

	glCheckError();

	_programs.push_back(program);

	model.hasProgram = true;
	model.program = static_cast<uint32_t>(_programs.size() - 1);

	_shaders[vertexShader + fragmentShader] = model.program;

	return true;
}

void Renderer::setCamera(uint64_t id) {
	if ((id && !_engine.entities.valid(id)) || (id && !_engine.entities.has<Transform>(id)))
		return;

	if (_camera)
		_engine.entities.dereference(id);

	if (id)
		_engine.entities.reference(id);

	_camera = id;
}

void Renderer::lockCursor(bool lock){
	if (lock)
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
