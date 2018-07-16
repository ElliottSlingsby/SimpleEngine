#include "Renderer.hpp"

Renderer::Renderer(Engine& engine, const ShaderVariables& shaderVariables) : _engine(engine), _shaderVariables(shaderVariables){
	SYSFUNC_ENABLE(SystemInterface, initiate, -1);
	SYSFUNC_ENABLE(SystemInterface, update, -1);
	SYSFUNC_ENABLE(SystemInterface, framebufferSize, 0);
	SYSFUNC_ENABLE(SystemInterface, textureLoaded, 0);
	SYSFUNC_ENABLE(SystemInterface, meshLoaded, 0);
}

void Renderer::initiate(const std::vector<std::string>& args){
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_MULTISAMPLE);
	//glEnable(GL_DITHER);

	//_windowSize = { height, width };
	//_projectionMatrix = glm::perspectiveFov(glm::radians(_fov), static_cast<double>(height), static_cast<double>(width), 1.0, _zDepth);

	//glViewport(0, 0, height, width);
}

void Renderer::update(double dt){
	/*
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_engine.iterateEntities([&](Engine::Entity& entity) {
		if (!entity.has<Transform, Model>())
			return;

		const Transform& transform = *entity.get<Transform>();
		const Model& model = *entity.get<Model>();
		const Material& material = *entity.get<Material>();

		glUseProgram(model.program);

		if (program.uniformTexture != -1)
			glUniform1i(program.uniformTexture, 0);

		// projection matrix
		if (program.uniformProjection != -1)
			glUniformMatrix4fv(program.uniformProjection, 1, GL_FALSE, &(static_cast<glm::mat4>(_projectionMatrix))[0][0]);

		// view matrix
		Mat4 viewMatrix = Renderer::viewMatrix();

		if (program.uniformView != -1)
			glUniformMatrix4fv(program.uniformView, 1, GL_FALSE, &(static_cast<glm::mat4>(viewMatrix))[0][0]);

		// model matrix
		Mat4 modelMatrix;

		if (program.uniformModelView != -1 || program.uniformView != -1) {
			modelMatrix = transform.matrix();

			if (program.uniformView != -1)
				glUniformMatrix4fv(program.uniformModel, 1, GL_FALSE, &(static_cast<glm::mat4>(modelMatrix))[0][0]);
		}

		// model view matrix
		if (program.uniformModelView != -1)
			glUniformMatrix4fv(program.uniformModelView, 1, GL_FALSE, &(static_cast<glm::mat4>(viewMatrix * modelMatrix))[0][0]);

		// set texture
		if (program.uniformTexture != -1 && model.texture) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, model.texture);

			if (model.linearTexture) {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}
		}

		// draw buffer
		if (model.arrayObject && model.attribBuffer && model.indexCount) {
			glBindVertexArray(model.arrayObject);
			glBindBuffer(GL_ARRAY_BUFFER, model.attribBuffer);

			glDrawArrays(GL_TRIANGLES, 0, model.indexCount);
		}

		// clean up
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	});
	*/

	SYSFUNC_CALL(SystemInterface, rendered, _engine)();
}

void Renderer::framebufferSize(glm::uvec3 size){

}

void Renderer::textureLoaded(uint64_t id, const std::string & file, const TextureData* textureData){

}

void Renderer::meshLoaded(uint64_t id, const std::string & file, const MeshData* meshData){

}

void Renderer::setShape(const ShapeConfig& config){
	_verticalFov = config.verticalFov;
	_zDepth = config.zDepth;
	_size = config.resolution;
}

void Renderer::loadProgram(const std::vector<std::tuple<GLuint, std::string>>& shaders, uint64_t id, bool reload){
	std::vector<GLuint> shaderIds;

	for (auto tuple : shaders) {
		GLuint shaderType = std::get<0>(tuple);
		const std::string& file = std::get<1>(tuple);

		//auto i = _shaders.find(file);

		//if (i != _shaders.end()) {
		//	shaderIds.push_back(i->second);
		//	break;
		//}

		// compile shader
		file;
		shaderType;

		shaderIds.push_back(0);
	}

	//auto i = _shaderPrograms.find(shaderIds);

	//if (i == _shaderPrograms.end())
	//	return i->second;

	// create new program
}
