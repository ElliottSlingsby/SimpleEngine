#include "Renderer.hpp"

#include "Transform.hpp"
#include "Model.hpp"

#include <iostream>
#include <string>

#include <tiny_obj_loader.h>
#include <stb_image.h>

GLint vertexAttribute = 0;
GLint normalAttribute = 1;
GLint texcoordAttribute = 2;

const std::string vertexShaderSrc =
	"#version 460 core\n"

	"layout (location = 0) in vec3 inVertex;"
	"layout (location = 1) in vec3 inNormal;"
	"layout (location = 2) in vec2 inTexcoord;"

	"out vec3 normal;"
	"out vec2 texcoord;"

	"uniform mat4 matrix;" 

	"void main(){"
		"gl_Position =  matrix * vec4(inVertex, 1);"
		"normal = inNormal;"
		"texcoord = inTexcoord;"
	"}";

const std::string fragmentShaderSrc = 
	"#version 460 core\n"

	"in vec3 normal;"
	"in vec2 texcoord;"

	"layout (location = 0) out vec4 fragColour;"

	"uniform sampler2D texture;"

	"void main(){"
		"fragColour = vec4(1, 0, 0, 1);"
	"}";

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

Renderer::Renderer(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Load, &Renderer::load);
	_engine.events.subscribe(this, Events::Update, &Renderer::update);
}

Renderer::~Renderer() {
	glfwTerminate();
}

void Renderer::load(int argc, char** argv) {
	// setup data stuff
	_path = upperPath(replace('\\', '/', argv[0])) + dataFolder + '/';
	
	_windowSize = { 512, 512 };
	_matrix = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.f, 1000.f);

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
	
	// create shaders
	bool failed = false;

	if (!compileShader(GL_VERTEX_SHADER, &_vertexShader, vertexShaderSrc))
		failed = true;

	if (!compileShader(GL_FRAGMENT_SHADER, &_fragmentShader, fragmentShaderSrc))
		failed = true;

	if (failed) {
		glDeleteShader(_fragmentShader);
		glDeleteShader(_vertexShader);
		glfwDestroyWindow(_window);
		_engine.events.unsubscribe(this);

		return;
	}

	glCheckError();

	_program = glCreateProgram();
	glAttachShader(_program, _vertexShader);
	glAttachShader(_program, _fragmentShader);

	glLinkProgram(_program);
	
	// link the program
	GLint success;
	glGetProgramiv(_program, GL_LINK_STATUS, &success);

	if (!success) {
		GLint length = 0;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> message(length);
		glGetProgramInfoLog(_program, length, &length, &message[0]);

		std::cerr << "GLSL error - " << static_cast<char*>(&message[0]) << std::endl;

		glDeleteShader(_vertexShader);
		glDeleteShader(_fragmentShader);
		glDeleteProgram(_program);
		glfwDestroyWindow(_window);
		_engine.events.unsubscribe(this);

		return;
	}

	// get attribute / uniform locations
	_uniformMatrix = glGetUniformLocation(_program, "matrix");
	_uniformTexture = glGetUniformLocation(_program, "texture");

	glCheckError();

	// create test entity
	uint64_t id = _engine.entities.create();
	_engine.entities.add<Transform>(id);
	_engine.entities.add<Model>(id);

	Model& model = *_engine.entities.get<Model>(id);

	// load texture data
	int x, y, n;
	uint8_t* imageData = stbi_load((_path + "image.png").c_str(), &x, &y, &n, 4);

	if (!imageData) {
		std::cerr << "cannot load texture" << std::endl;
		return;
	}

	stbi_image_free(imageData);

	// load model data
	tinyobj::attrib_t attributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string error;
	tinyobj::LoadObj(&attributes, &shapes, &materials, &error, (_path + "triangle.obj").c_str());

	if (!error.empty()) {
		std::cerr << error << std::endl;
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
	GLuint* indexMap = static_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));
	Attributes* attributeMap = static_cast<Attributes*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

	glCheckError();

	for (const tinyobj::shape_t& shape : shapes) {
		for (uint32_t i = 0; i < shape.mesh.indices.size(); i++) {
			const tinyobj::index_t& index = shape.mesh.indices[i];

			indexMap[i] = index.vertex_index;

			if (attributes.vertices.size())
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
	glEnableVertexAttribArray(vertexAttribute);
	glVertexAttribPointer(vertexAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)(0));

	glEnableVertexAttribArray(normalAttribute);
	glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)(sizeof(Attributes::vertex)));

	glEnableVertexAttribArray(texcoordAttribute);
	glVertexAttribPointer(texcoordAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Attributes), (void*)(sizeof(Attributes::vertex) + sizeof(Attributes::normal)));

	glCheckError();
	
	// clean up
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glCheckError();
}

void Renderer::update(double dt) {
	if (glfwWindowShouldClose(_window))
		_engine.running = false;

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(_program);

	glUniformMatrix4fv(_uniformMatrix, 1, GL_FALSE, &_matrix[0][0]);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glCheckError();	

	_engine.entities.iterate<Transform, Model>([&](uint64_t id, Transform& transform, Model& model) {
		glBindVertexArray(model.arrayObject);
		glBindBuffer(GL_ARRAY_BUFFER, model.attribBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.indexBuffer);

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