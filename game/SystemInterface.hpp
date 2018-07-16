#pragma once

#include <SimpleEngine.hpp>

#include <glm\glm.hpp>
#include <vector>

class SystemInterface : public SimpleEngine<SystemInterface, 32>::BaseSystem {
protected:
	enum Modifiers {
		Shift,
		Control,
		Alt,
		Super
	};

	enum Action {
		Press,
		Release,
		Repeat
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

	struct AnimationData {

	};

public:
	virtual void initiate(const std::vector<std::string>& args) {}
	virtual void update(double dt) {}

	virtual void physicsUpdate(double timescale) {}
	virtual void collision(Engine::Entity& first, Engine::Entity& second) {}

	virtual void rendered() {}

	virtual void scrollWheel(glm::dvec3 offset) {}
	virtual void mousePress(uint32_t button, Action action, Modifiers mods) {}
	virtual void textInput(uint32_t utf32, Modifiers mods) {}
	virtual void fileDrop(const std::vector<std::string>& paths) {}
	virtual void cursorEnter(bool entered) {}
	virtual void cursorPosition(glm::dvec3 position) {}
	virtual void windowPosition(glm::uvec3 position) {}
	virtual void framebufferSize(glm::uvec3 size) {}
	virtual void windowSize(glm::uvec3 size) {}
	virtual void windowOpen(bool opened) {}

	virtual void textureLoaded(uint64_t id, const std::string& file, const TextureData* textureData) {}
	virtual void meshLoaded(uint64_t id, const std::string& file, const MeshData* meshData) {}
	virtual void animationLoaded(uint64_t id, const std::string& file, const AnimationData* animationData) {}
};