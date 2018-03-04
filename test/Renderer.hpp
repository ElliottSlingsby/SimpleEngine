#pragma once

#include "Config.hpp"
#include "Transform.hpp"
#include "Model.hpp"

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\gtc\matrix_transform.hpp>

class Renderer {
	Engine& _engine;

public:
	Renderer(Engine& engine);

	void load(int argc, char** argv);
	void update(double dt);
};

Renderer::Renderer(Engine& engine) : _engine(engine) {
	_engine.events.subscribe(this, Events::Load, &Renderer::load);
	_engine.events.subscribe(this, Events::Update, &Renderer::update);
}

void Renderer::load(int argc, char** argv) {

}

void Renderer::update(double dt) {

}