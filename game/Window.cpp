#include "Window.hpp"

#define ENGINE_REF (*(SystemInterface::Engine*)(glfwGetWindowUserPointer(window)))

void mousePressCallback(GLFWwindow* window, int button, int action, int mods) {
	SYSFUNC_CALL(SystemInterface, mousePress, ENGINE_REF)(button, (SystemInterface::Action)action, (SystemInterface::Modifier)mods);
}

void scrollWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {
	SYSFUNC_CALL(SystemInterface, scrollWheel, ENGINE_REF)(glm::dvec2(xoffset, yoffset));
}

void fileDropCallback(GLFWwindow* window, int count, const char** paths) {
	SYSFUNC_CALL(SystemInterface, fileDrop, ENGINE_REF)(std::vector<std::string>(paths, paths + count));
}

void cursorEnterCallback(GLFWwindow* window, int entered) {
	SYSFUNC_CALL(SystemInterface, cursorEnter, ENGINE_REF)(entered);
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
	SYSFUNC_CALL(SystemInterface, cursorPosition, ENGINE_REF)(glm::dvec2(xpos, ypos));
}

void textInputCallback(GLFWwindow* window, unsigned int codepoint, int mods) {
	SYSFUNC_CALL(SystemInterface, textInput, ENGINE_REF)(codepoint, (SystemInterface::Modifier)mods);
}

void keyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	SYSFUNC_CALL(SystemInterface, keyInput, ENGINE_REF)((key < 0) ? 0 : (uint32_t)key, (SystemInterface::Action)action, (SystemInterface::Modifier)mods);
}

void windowPositionCallback(GLFWwindow* window, int xpos, int ypos) {
	SYSFUNC_CALL(SystemInterface, windowPosition, ENGINE_REF)(glm::uvec2(xpos, ypos));
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	SYSFUNC_CALL(SystemInterface, framebufferSize, ENGINE_REF)(glm::uvec2(width, height));
}

void windowSizeCallback(GLFWwindow* window, int width, int height) {
	SYSFUNC_CALL(SystemInterface, windowSize, ENGINE_REF)(glm::uvec2(width, height));
}

/*
void windowCloseCallback(GLFWwindow* window) {
	//SYSFUNC_CALL(SystemInterface, windowOpen, ENGINE_REF)(false);
}
*/

void errorCallback(int error, const char* description) {
	std::cerr << error << std::endl << description << std::endl << std::endl;
}

Window::Window(Engine& engine, const ConstructorInfo& constructorInfo) : _engine(engine), _constructorInfo(constructorInfo){
	SYSFUNC_ENABLE(SystemInterface, initiate, -1);
	SYSFUNC_ENABLE(SystemInterface, update, -1);

	SYSFUNC_ENABLE(SystemInterface, rendered, 0);
}

Window::~Window(){
	glfwTerminate();
}

void Window::initiate(const std::vector<std::string>& args){
	glfwSetErrorCallback(errorCallback);

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, (int)_constructorInfo.contextVersionMajor);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, (int)_constructorInfo.contextVersionMinor);

	if (_constructorInfo.coreContex)
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (int)_constructorInfo.debugContext);

	glfwWindowHint(GLFW_RESIZABLE, (int)hasFlags(_constructorInfo.defualtModes, Mode::Resizable));
	glfwWindowHint(GLFW_MAXIMIZED, (int)hasFlags(_constructorInfo.defualtModes, Mode::Maximised));
	glfwWindowHint(GLFW_DECORATED, (int)hasFlags(_constructorInfo.defualtModes, Mode::Decorated));

	glfwWindowHint(GLFW_SAMPLES, (int)_constructorInfo.defaultSuperSampling);

	//glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_NONE);

	_window = glfwCreateWindow((int)_constructorInfo.defualtSize.x, (int)_constructorInfo.defualtSize.y, _constructorInfo.defualtTitle.c_str(), nullptr, nullptr);

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	if (hasFlags(_constructorInfo.defualtModes, Mode::VerticalSync))
		glfwSwapInterval(1);

	glfwSetWindowUserPointer(_window, &_engine);

	glfwSetMouseButtonCallback(_window, mousePressCallback);
	glfwSetScrollCallback(_window, scrollWheelCallback);
	glfwSetDropCallback(_window, fileDropCallback);
	glfwSetCursorEnterCallback(_window, cursorEnterCallback);
	glfwSetCursorPosCallback(_window, cursorPositionCallback);
	glfwSetCharModsCallback(_window, textInputCallback);
	glfwSetKeyCallback(_window, keyInputCallback);
	glfwSetWindowPosCallback(_window, windowPositionCallback);
	glfwSetFramebufferSizeCallback(_window, framebufferSizeCallback);
	glfwSetWindowSizeCallback(_window, windowSizeCallback);
	//glfwSetWindowCloseCallback(_window, windowCloseCallback);

	SYSFUNC_CALL(SystemInterface, windowSize, _engine)(_constructorInfo.defualtSize);
	
	int x, y;
	glfwGetFramebufferSize(_window, &x, &y);

	SYSFUNC_CALL(SystemInterface, framebufferSize, _engine)(glm::uvec2(x, y));
	//SYSFUNC_CALL(SystemInterface, windowOpen, _engine)(true);
}

void Window::update(double dt){
	if (!_window)
		return;

	glfwPollEvents();

	if (glfwWindowShouldClose(_window)) {
		//_engine.quit();

		glfwDestroyWindow(_window);

		initiate({});


	}
}

void Window::rendered(){
	if (!_window)
		return;

	glfwSwapBuffers(_window);
}

void Window::enableModes(uint8_t modes){
	assert(_window);

	if (hasFlags(modes, Mode::LockedCursor))
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (hasFlags(modes, Mode::VerticalSync))
		glfwSwapInterval(1);

}

void Window::disableModes(uint8_t modes){
	assert(_window);

	if (hasFlags(modes, Mode::LockedCursor))
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (hasFlags(modes, Mode::VerticalSync))
		glfwSwapInterval(0);
}

/*
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

	glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_NONE);

	_window = glfwCreateWindow((int)config.windowSize.x, (int)config.windowSize.y, config.windowTitle.c_str(), nullptr, _window);

	glfwMakeContextCurrent(_window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	if (hasFlags(config.flags, WindowConfig::VerticalSync))
		glfwSwapInterval(1);

	glfwSetWindowUserPointer(_window, &_engine);

	glfwSetMouseButtonCallback(_window, mousePressCallback);
	glfwSetScrollCallback(_window, scrollWheelCallback);
	glfwSetDropCallback(_window, fileDropCallback);
	glfwSetCursorEnterCallback(_window, cursorEnterCallback);
	glfwSetCursorPosCallback(_window, cursorPositionCallback);
	glfwSetCharModsCallback(_window, textInputCallback);
	glfwSetKeyCallback(_window, keyInputCallback);
	glfwSetWindowPosCallback(_window, windowPositionCallback);
	glfwSetFramebufferSizeCallback(_window, framebufferSizeCallback);
	glfwSetWindowSizeCallback(_window, windowSizeCallback);
	glfwSetWindowCloseCallback(_window, windowCloseCallback);

	SYSFUNC_CALL(SystemInterface, windowSize, _engine)(config.windowSize);
	SYSFUNC_CALL(SystemInterface, framebufferSize, _engine)(config.windowSize);
	SYSFUNC_CALL(SystemInterface, windowOpen, _engine)(true);
}

void Window::closeWindow(){
	if (!_window)
		return;

	glfwDestroyWindow(_window);
}

void Window::lockCursor(bool locked){
	if (!_window)
		return;

	if (locked)
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
*/