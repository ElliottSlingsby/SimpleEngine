#include "Window.hpp"

#define ENGINE_REF (*(SystemInterface::Engine*)(glfwGetWindowUserPointer(window)))

void mousePressCallback(GLFWwindow* window, int button, int action, int mods) {
	//SYSFUNC_CALL(SystemInterface, mousePress, ENGINE_REF)(button, action, mods);
}

void keyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//SYSFUNC_CALL(SystemInterface, keyPress, ENGINE_REF)(key, scancode, action, mods);
}

void scrollWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	//SYSFUNC_CALL(SystemInterface, scrollWheel, ENGINE_REF)(xoffset, yoffset);
}

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
	//SYSFUNC_CALL(SystemInterface, fileDrop, ENGINE_REF)(count, paths);
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
	//SYSFUNC_CALL(SystemInterface, cursorEnter, ENGINE_REF)(entered);
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	//SYSFUNC_CALL(SystemInterface, cursorPosition, ENGINE_REF)(xpos, ypos);
}

void textInputCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	//SYSFUNC_CALL(SystemInterface, textInput, ENGINE_REF)(codepoint, mods);
}

void windowPositionCallback(GLFWwindow* window, int xpos, int ypos) {
	//SYSFUNC_CALL(SystemInterface, windowPosition, ENGINE_REF)(xpos, ypos);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	//SYSFUNC_CALL(SystemInterface, framebufferSize, ENGINE_REF)(width, height);
}

void windowSizeCallback(GLFWwindow* window, int width, int height) {
	//SYSFUNC_CALL(SystemInterface, windowSize, ENGINE_REF)(width, height);
}

void windowCloseCallback(GLFWwindow* window) {
	//SYSFUNC_CALL(SystemInterface, windowClose, ENGINE_REF)();
}

Window::Window(Engine& engine) : _engine(engine){
	SYSFUNC_ENABLE(SystemInterface, initiate, -1);
	SYSFUNC_ENABLE(SystemInterface, rendered, 0);
}

Window::~Window(){
	glfwTerminate();
}

void Window::initiate(const std::vector<std::string>& args){
	glfwInit();
}

void Window::update(double dt){
	glfwPollEvents();

	if (glfwWindowShouldClose(_window))
		closeWindow();
}

void Window::rendered(){
	if (!_window)
		return;

	glfwSwapBuffers(_window);
}

void Window::openWindow(const WindowConfig& config){
	closeWindow();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)config.contextVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)config.contextVersionMinor);

	if (hasFlags(config.flags, WindowConfig::CoreContext))
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (int)hasFlags(config.flags, WindowConfig::DebugContext));
	
	glfwWindowHint(GLFW_RESIZABLE, (int)hasFlags(config.flags, WindowConfig::WindowResizable));
	glfwWindowHint(GLFW_MAXIMIZED, (int)hasFlags(config.flags, WindowConfig::WindowMaximised));
	glfwWindowHint(GLFW_DECORATED, (int)hasFlags(config.flags, WindowConfig::WindowDecorated));

	glfwWindowHint(GLFW_SAMPLES, (int)config.superSampling);

	_window = glfwCreateWindow((int)config.windowSize.x, (int)config.windowSize.y, config.windowTitle.c_str(), nullptr, nullptr);

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	if (hasFlags(config.flags, WindowConfig::VerticalSync))
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

	//SYSFUNC_CALL(SystemInterface, windowOpen, _engine)();
	SYSFUNC_ENABLE(SystemInterface, update, -1);
}

void Window::closeWindow(){
	if (!_window)
		return;

	glfwDestroyWindow(_window);
	_window = nullptr;

	SYSFUNC_DISABLE(SystemInterface, update);
}