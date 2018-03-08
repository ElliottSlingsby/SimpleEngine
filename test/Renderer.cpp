#include "Renderer.hpp"

#include "Transform.hpp"
#include "Model.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <tiny_obj_loader.h>
#include <stb_image.h>

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
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
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

Renderer::Renderer(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Load, &Renderer::load);
	_engine.events.subscribe(this, Events::Update, &Renderer::update);
}

Renderer::~Renderer() {
	glfwTerminate();
}

void Renderer::load(int argc, char** argv) {
	// setup data stuff
	_path = upperPath(replace('\\', '/', argv[0])) + DATA_FOLDER + '/';
	
	_windowSize = { 512, 512 };
	_matrix = glm::ortho(-1.f, 1.f, -1.f, 1.f);

	stbi_set_flip_vertically_on_load(true);
	
	// setup GLFW
	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		_engine.events.unsubscribe(this);
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	_window = glfwCreateWindow(_windowSize.x, _windowSize.y, "", nullptr, nullptr);

	if (!_window) {
		std::cerr << "GLFW error - " << "cannot create window" << std::endl;
		_engine.events.unsubscribe(this);
		return;
	}

	glfwSetKeyCallback(_window, keyCallback);

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	if (!createProgram(&_program, &_vertexShader, &_fragmentShader, readFile(_path + VERTEX_SHADER_FILE), readFile(_path + FRAGMENT_SHADER_FILE))) {
		glfwDestroyWindow(_window);
		_engine.events.unsubscribe(this);

		return;
	}

	_uniformMatrix = glGetUniformLocation(_program, "matrix");
	_uniformTexture = glGetUniformLocation(_program, "texture");

	glCheckError();
}

void Renderer::update(double dt) {
	if (glfwWindowShouldClose(_window))
		_engine.running = false;

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(_program);

	if (_uniformMatrix != -1)
		glUniformMatrix4fv(_uniformMatrix, 1, GL_FALSE, &_matrix[0][0]);

	if (_uniformTexture != -1)
		glUniform1i(_uniformTexture, 0);

	glCheckError();

	_engine.entities.iterate<Transform, Model>([&](uint64_t id, Transform& transform, Model& model) {
		glBindVertexArray(model.arrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, model.attribBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model.texture);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(model.indexCount * 3), GL_UNSIGNED_INT, nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glCheckError();
	});

	glUseProgram(0);
	glCheckError();

	glfwSwapBuffers(_window);
	glfwPollEvents();
}

void Renderer::loadMesh(uint64_t* id, const std::string& meshFile) {
	if (!_engine.entities.valid(*id))
		return;

	_engine.entities.add<Model>(*id);
	Model& model = *_engine.entities.get<Model>(*id);

	// load model data
	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string error;
	tinyobj::LoadObj(&attributes, &shapes, &materials, &error, (_path + meshFile).c_str());

	if (!error.empty()) {
		std::cerr << "cannot load mesh - " << _path + meshFile << std::endl;
		return;
	}

	if (!attributes.vertices.size()) {
		std::cerr << "problem reading vertex data - " << _path + meshFile << std::endl;
		return;
	}

	for (const tinyobj::shape_t& shape : shapes)
		model.indexCount += shape.mesh.indices.size();

	// create buffers
	glGenVertexArrays(1, &model.arrayObject);
	glBindVertexArray(model.arrayObject);

	glGenBuffers(1, &model.indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indexCount * sizeof(GLuint), nullptr, GL_STATIC_DRAW);

	glGenBuffers(1, &model.attribBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model.attribBuffer);
	glBufferData(GL_ARRAY_BUFFER, (attributes.vertices.size() / 3) * sizeof(Attributes), nullptr, GL_STATIC_DRAW);

	// buffer data
	GLuint* indexMap = static_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
	Attributes* attributeMap = static_cast<Attributes*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

	glCheckError();

	for (const tinyobj::shape_t& shape : shapes) {
		for (uint32_t i = 0; i < shape.mesh.indices.size(); i++) {
			const tinyobj::index_t& index = shape.mesh.indices[i];

			indexMap[i] = index.vertex_index;

			memcpy(&attributeMap[index.vertex_index].vertex, &attributes.vertices[(index.vertex_index * 3)], sizeof(Attributes::vertex));

			if (attributes.normals.size())
				memcpy(&attributeMap[index.vertex_index].normal, &attributes.normals[(index.normal_index * 3)], sizeof(Attributes::normal));

			if (attributes.texcoords.size())
				memcpy(&attributeMap[index.vertex_index].texcoord, &attributes.texcoords[(index.texcoord_index * 2)], sizeof(Attributes::texcoord));
		}
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

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
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glCheckError();
}

void Renderer::loadTexture(uint64_t* id, const std::string& textureFile) {
	if (!_engine.entities.valid(*id))
		return;

	_engine.entities.add<Model>(*id);
	Model& model = *_engine.entities.get<Model>(*id);

	// load data
	int x, y, n;
	uint8_t* data = stbi_load((_path + textureFile).c_str(), &x, &y, &n, 4);

	if (!data) {
		std::cerr << "cannot load image - " << _path + textureFile << std::endl;
		return;
	}

	// buffer data
	glGenTextures(1, &model.texture);
	glBindTexture(GL_TEXTURE_2D, model.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	glCheckError();

	stbi_image_free(data);
}