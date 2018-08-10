#pragma once

#include <SimpleEngine.hpp>

#include <glm\glm.hpp>
#include <vector>

class SystemInterface : public SimpleEngine<SystemInterface, 32>::BaseSystem {
public:
	enum Modifier : uint8_t {
		Mod_None = 0,
		Mod_Shift = 1,
		Mod_Ctrl = 2,
		Mod_Alt = 4,
		Mod_Caps = 8,
	};

	enum Action {
		Release,
		Press,
	};

	enum Key {
		Key_Unknown,
		Key_Space,
		Key_Quote,
		Key_Comma,
		Key_Minus,
		Key_Period,
		Key_Slash,
		Key_0,
		Key_1,
		Key_2,
		Key_3,
		Key_4,
		Key_5,
		Key_6,
		Key_7,
		Key_8,
		Key_9,
		Key_Semicolon,
		Key_Equal,
		Key_A,
		Key_B,
		Key_C,
		Key_D,
		Key_E,
		Key_F,
		Key_G,
		Key_H,
		Key_I,
		Key_J,
		Key_K,
		Key_L,
		Key_M,
		Key_N,
		Key_O,
		Key_P,
		Key_Q,
		Key_R,
		Key_S,
		Key_T,
		Key_U,
		Key_V,
		Key_W,
		Key_X,
		Key_Y,
		Key_Z,
		Key_LBracket,
		Key_Backslash,
		Key_RBracket,
		Key_GraveAccent,
		Key_Escape,
		Key_Enter,
		Key_Tab,
		Key_Backspace,
		Key_Insert,
		Key_Delete,
		Key_Right,
		Key_Left,
		Key_Down,
		Key_Up,
		Key_PageUp,
		Key_PageDown,
		Key_Home,
		Key_End,
		Key_CapsLock,
		Key_ScrollLock,
		Key_NumLock,
		Key_PrintScreen,
		Key_Pause,
		Key_F1,
		Key_F2,
		Key_F3,
		Key_F4,
		Key_F5,
		Key_F6,
		Key_F7,
		Key_F8,
		Key_F9,
		Key_F10,
		Key_F11,
		Key_F12,
		Key_F13,
		Key_F14,
		Key_F15,
		Key_F16,
		Key_F17,
		Key_F18,
		Key_F19,
		Key_F20,
		Key_F21,
		Key_F22,
		Key_F23,
		Key_F24,
		Key_Num0,
		Key_Num1,
		Key_Num2,
		Key_Num3,
		Key_Num4,
		Key_Num5,
		Key_Num6,
		Key_Num7,
		Key_Num8,
		Key_Num9,
		Key_NumDecimal,
		Key_NumDivide,
		Key_NumMultiply,
		Key_NumSubtract,
		Key_NumAdd,
		Key_NumEnter,
		Key_NumEqual,
		Key_LShift,
		Key_LCtrl,
		Key_LAlt,
		Key_RShift,
		Key_RCtrl,
		Key_RAlt,
		Key_Menu
	};

	struct TextureData {
		uint32_t channels;
		glm::uvec2 size;
		std::vector<glm::tvec4<uint8_t>> colours;
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

	struct AnimationData {

	};

	virtual void initiate(const std::vector<std::string>& args) {}
	virtual void preUpdate(double dt) {}
	virtual void update(double dt) {}

	//virtual void physicsUpdate(double timescale) {}
	//virtual void collision(uint64_t first, uint64_t second) {}

	//virtual void rendered() {}

	virtual void render() {}

	virtual void scrollWheel(glm::dvec2 offset) {}
	virtual void mousePress(uint32_t button, Action action, uint8_t mods) {}
	virtual void textInput(uint32_t utf32, uint8_t mods) {}
	virtual void keyInput(uint32_t key, Action action, uint8_t mods) {}
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
};