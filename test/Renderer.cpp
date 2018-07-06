#include "Renderer.hpp"

Renderer::Renderer(Engine& engine) : _engine(engine){
	SYSFUNC_ENABLE(SystemInterface, initiate, -1);
	SYSFUNC_ENABLE(SystemInterface, update, -1);
	SYSFUNC_ENABLE(SystemInterface, framebufferSize, 0);
}

void Renderer::initiate(int argc, char ** argv){

}

void Renderer::update(double dt){
	_engine.iterateEntities([&](Engine::Entity& entity) {
		Model* model = entity.get<Model>();
		Material* material = entity.get<Material>();
	
		if (!model)
			return;
	
	
	});

	_engine.system<Window>().swapBuffer();
}

void Renderer::framebufferSize(int width, int height){

}

uint32_t Renderer::createProgram(const std::vector<std::tuple<ShaderType, std::string>> shaders){
	return 0;
}
