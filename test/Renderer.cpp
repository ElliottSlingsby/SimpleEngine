#include "Renderer.hpp"

#include "Transform.hpp"
#include "Model.hpp"

#include <iostream>
#include <string>

#include <tiny_obj_loader.h>
#include <stb_image.h>

const std::string vertexShaderSrc =
	"#version 460 core\n"

	"layout (location = 0) in vec3 inVertex;"
	"layout (location = 1) in vec2 inTexcoord;"

	"varying out vec2 texcoord;"

	"uniform mat4 matrix;"

	"void main(){"
		"gl_Position =  matrix * vec4(inVertex, 1);"
		"texcoord = inTexcoord;"
	"}";

const std::string fragmentShaderSrc = 
	"#version 460 core\n"

	"varying in vec2 texcoord;"

	"layout (location = 0) varying out vec4 fragColour;"

	"uniform sampler2D texture;"

	"void main(){"
		"fragColour = texture2D(texture, texcoord).rgba;"
	"}";


int verboseCheckError() {
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return 0;

	std::cout << error << std::endl;

	return 1;
}

#define glCheckError() assert(verboseCheckError() == 0)

void errorCallback(int error, const char* description) {
	std::cout << "GLFW Error - " << error << " - " << description << std::endl;
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

	std::cout << "GLSL error - " << static_cast<char*>(&message[0]) << std::endl;
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
	_path = upperPath(replace('\\', '/', argv[0]));

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		_engine.events.unsubscribe(this, Events::Update);
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	_window = glfwCreateWindow(512, 512, "SimpleEngine Renderer", nullptr, nullptr);

	if (!_window) {
		std::cout << "GLFW error - " << "cannot create window" << std::endl;
		_engine.events.unsubscribe(this, Events::Update);
		return;
	}

	glfwSetKeyCallback(_window, keyCallback);

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	bool failed = false;

	if (!compileShader(GL_VERTEX_SHADER, &_vertexShader, vertexShaderSrc))
		failed = true;

	if (!compileShader(GL_FRAGMENT_SHADER, &_fragmentShader, fragmentShaderSrc))
		failed = true;

	if (failed) {
		glDeleteShader(_fragmentShader);
		glDeleteShader(_vertexShader);
		glfwDestroyWindow(_window);
		_engine.events.unsubscribe(this, Events::Update);

		return;
	}

	glCheckError();

	_program = glCreateProgram();
	glAttachShader(_program, _vertexShader);
	glAttachShader(_program, _fragmentShader);

	glLinkProgram(_program);

	GLint success;
	glGetProgramiv(_program, GL_LINK_STATUS, &success);

	if (!success) {
		GLint length = 0;
		glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &length);

		std::vector<GLchar> message(length);
		glGetProgramInfoLog(_program, length, &length, &message[0]);

		std::cout << "GLSL error - " << static_cast<char*>(&message[0]) << std::endl;

		glDeleteShader(_vertexShader);
		glDeleteShader(_fragmentShader);
		glDeleteProgram(_program);
		glfwDestroyWindow(_window);
		_engine.events.unsubscribe(this, Events::Update);

		return;
	}

	glCheckError();

	GLuint _attributeVertex = glGetAttribLocation(_program, "inVertex");
	GLuint _attributeTexcoord = glGetAttribLocation(_program, "inNormal");
	GLuint _uniformMatrix = glGetUniformLocation(_program, "matrix");
	GLuint _uniformTexture = glGetUniformLocation(_program, "texture");

	glCheckError();

	//glGenBuffers(1, &state->vertexBuffer); // glGenBuffers
	//glGenBuffers(1, &state->indexBuffer); // glGenBuffers
	//glCheckError();
	//
	//glBindBuffer(GL_ARRAY_BUFFER, state->vertexBuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->indexBuffer);
	//
	//glVertexAttribPointer(state->attributePosition, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	//glEnableVertexAttribArray(state->attributePosition);
	//glCheckError();
	//
	//glVertexAttribPointer(state->attributeUv, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
	//glEnableVertexAttribArray(state->attributeUv);
	//glCheckError();
	//
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Renderer::update(double dt) {
	if (glfwWindowShouldClose(_window))
		_engine.running = false;

	_engine.entities.iterate<Transform, Model>([&](uint64_t id, Transform& transform, Model& model) {

	});

	glfwSwapBuffers(_window);
	glfwPollEvents();
}