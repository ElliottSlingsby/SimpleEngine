#pragma once

#include <SimpleEngine.hpp>

#include <glm\glm.hpp>
#include <vector>

class SystemInterface : public SimpleEngine<SystemInterface, 32>::BaseSystem {
public:
	enum Modifier : uint8_t {
		Shift = 1,
		Ctrl = 2,
		Alt = 4,
		Super = 8
	};

	enum Action : uint8_t {
		Release = 0,
		Press = 1,
		Repeat = 2
	};

	enum Key : uint32_t {
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Delete = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PageUp = 266,
		PageDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		Num0 = 320,
		Num1 = 321,
		Num2 = 322,
		Num3 = 323,
		Num4 = 324,
		Num5 = 325,
		Num6 = 326,
		Num7 = 327,
		Num8 = 328,
		Num9 = 329,
		NumDecimal = 330,
		NumDivide = 331,
		NumMultiply = 332,
		NumSubtract = 333,
		NumAdd = 334,
		NumEnter = 335,
		NumEqual = 336,
		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348
	};

	struct TextureData {
		uint32_t channels;
		glm::uvec2 size;
		std::vector<uint8_t> colours;
	};

	struct MeshData {
		struct Vertex {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texcoord;
			glm::vec4 colour;
			glm::vec3 tangent;
			glm::vec3 bitangent;
		};

		std::vector<uint32_t> indexes;
		std::vector<Vertex> vertices;
	};

	struct ShaderData {
		enum Type {
			GeometryShader,
			VertexShader,
			FragmentShader
		};

		std::string source;
		Type type;
	};

	struct ProgramData {
		std::string geometryShader;
		std::string vertexShader;
		std::string fragmentShader;
	};

	struct AnimationData {

	};

	virtual void initiate(const std::vector<std::string>& args) {}
	virtual void update(double dt) {}

	//virtual void physicsUpdate(double timescale) {}
	//virtual void collision(uint64_t first, uint64_t second) {}

	virtual void rendered() {}

	virtual void scrollWheel(glm::dvec2 offset) {}
	virtual void mousePress(uint32_t button, Action action, Modifier mods) {}
	virtual void textInput(uint32_t utf32, Modifier mods) {}
	virtual void keyInput(uint32_t key, Action action, Modifier mods) {}
	virtual void fileDrop(const std::vector<std::string>& paths) {}
	virtual void cursorEnter(bool entered) {}
	virtual void cursorPosition(glm::dvec2 position) {}
	virtual void windowPosition(glm::uvec2 position) {}
	virtual void framebufferSize(glm::uvec2 size) {}
	virtual void windowSize(glm::uvec2 size) {}
	virtual void windowOpen(bool opened) {}

	virtual void textureLoaded(uint64_t id, const std::string& file, const TextureData* textureData) {}
	virtual void meshLoaded(uint64_t id, const std::string& file, const MeshData* meshData) {}
	//virtual void animationLoaded(uint64_t id, const std::string& file, const AnimationData* animationData) {}

	//virtual void shaderLoaded(uint64_t id, const std::string& file, const ShaderData* shaderData) {}
	//virtual void programLoaded(uint64_t id, const std::string& file, const ProgramData* programData) {}
};