#include "Window.hpp"

#define ENGINE_REF (*(SystemInterface::Engine*)(glfwGetWindowUserPointer(window)))

void mousePressCallback(GLFWwindow* window, int button, int action, int mods) {
	SYSFUNC_CALL(SystemInterface, mousePress, ENGINE_REF)(button, action, mods);
}

void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	SYSFUNC_CALL(SystemInterface, keyPress, ENGINE_REF)(key, scancode, action, mods);
}

void scrollWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	SYSFUNC_CALL(SystemInterface, scrollWheel, ENGINE_REF)(xoffset, yoffset);
}

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
	SYSFUNC_CALL(SystemInterface, fileDrop, ENGINE_REF)(count, paths);
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
	SYSFUNC_CALL(SystemInterface, cursorEnter, ENGINE_REF)(entered);
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	SYSFUNC_CALL(SystemInterface, cursorPosition, ENGINE_REF)(xpos, ypos);
}

void textInputCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	SYSFUNC_CALL(SystemInterface, textInput, ENGINE_REF)(codepoint, mods);
}

void windowPositionCallback(GLFWwindow* window, int xpos, int ypos) {
	SYSFUNC_CALL(SystemInterface, windowPosition, ENGINE_REF)(xpos, ypos);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	SYSFUNC_CALL(SystemInterface, framebufferSize, ENGINE_REF)(width, height);
}

void windowSizeCallback(GLFWwindow* window, int width, int height) {
	SYSFUNC_CALL(SystemInterface, windowSize, ENGINE_REF)(width, height);
}

void windowCloseCallback(GLFWwindow* window) {
	SYSFUNC_CALL(SystemInterface, windowClose, ENGINE_REF)();
}

Window::Window(Engine& engine) : _engine(engine){
	SYSFUNC_ENABLE(SystemInterface, initiate, -1);
}

Window::~Window(){
	glfwTerminate();
}

void Window::initiate(int argc, char ** argv){
	glfwInit();
}

void Window::update(double dt){
	glfwPollEvents();

	if (glfwWindowShouldClose(_window))
		close();
}

void Window::open(const Config& config){
	close();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)config.glContextVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)config.glContextVersionMinor);

	if (config.glCoreContext)
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (config.glDebugContext)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	
	glfwWindowHint(GLFW_RESIZABLE, (int)hasFlags(config.mode, Config::Resizable));
	glfwWindowHint(GLFW_MAXIMIZED, (int)hasFlags(config.mode, Config::Maximised));
	glfwWindowHint(GLFW_DECORATED, (int)hasFlags(config.mode, Config::Decorated));

	glfwWindowHint(GLFW_SAMPLES, (int)config.superSampling);

	_window = glfwCreateWindow((int)config.width, (int)config.height, config.title.c_str(), nullptr, nullptr);

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	if (config.vSync)
		glfwSwapInterval(1);

	glfwSetWindowUserPointer(_window, &_engine);

	glfwSetMouseButtonCallback(_window, mousePressCallback);
	glfwSetKeyCallback(_window, keyPressCallback);
	glfwSetScrollCallback(_window, scrollWheelCallback);

	glfwSetDropCallback(_window, fileDropCallback);
	glfwSetCursorEnterCallback(_window, cursorEnterCallback);
	glfwSetCursorPosCallback(_window, cursorPositionCallback);
	glfwSetCharModsCallback(_window, textInputCallback);

	glfwSetWindowPosCallback(_window, windowPositionCallback);
	glfwSetFramebufferSizeCallback(_window, framebufferSizeCallback);
	glfwSetWindowSizeCallback(_window, windowSizeCallback);
	glfwSetWindowCloseCallback(_window, windowCloseCallback);

	SYSFUNC_ENABLE(SystemInterface, update, -1);
}

void Window::close(){
	if (!_window)
		return;

	glfwDestroyWindow(_window);
	_window = nullptr;

	SYSFUNC_DISABLE(SystemInterface, update);
}

void Window::swapBuffer(){
	if (!_window)
		return;

	glfwSwapBuffers(_window);
}