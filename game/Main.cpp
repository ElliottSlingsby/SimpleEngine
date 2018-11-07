#include "SystemInterface.hpp"

#include "Transform.hpp"

#include "Window.hpp"
#include "Renderer.hpp"
#include "Controller.hpp"

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\matrix_decompose.hpp>
#include <glm\geometric.hpp>

#include <algorithm>

/*
	- make uv interpolation a per triangle function, and run for entire scene
	- fix hierarchical scaling
	- prevent multiple loading of same meshes/hierarchies in renderer
*/

void recursivelySetTexture(SystemInterface::Engine& engine, uint64_t id, GLuint textureBufferId) {
	Transform* transform = engine.getComponent<Transform>(id);
	Model* model = engine.getComponent<Model>(id);

	if (model)
		model->textureBufferId = textureBufferId;

	if (!transform || !transform->hasChildren())
		return;

	std::vector<uint64_t> children;
	transform->getChildren(&children);

	for (uint64_t i : children)
		recursivelySetTexture(engine, i, textureBufferId);
}

uint64_t recursivelyFindName(SystemInterface::Engine& engine, uint64_t id, const std::string& name) {
	Model* model = engine.getComponent<Model>(id);

	if (model && model->meshName == name)
		return id;

	Transform* transform = engine.getComponent<Transform>(id);

	if (!transform || !transform->hasChildren())
		return 0;

	std::vector<uint64_t> children;
	transform->getChildren(&children);

	for (uint64_t i : children) {
		uint64_t found = recursivelyFindName(engine, i, name);
	
		if (found)
			return found;
	}

	return 0;
}

template <typename T>
T crossProduct(const glm::tvec2<T>& a, const glm::tvec2<T>& b) {
	return glm::cross(glm::tvec3<T>(a, 0), glm::tvec3<T>(b, 0)).z;
}

glm::vec3 baryInterpolate(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, float s, float t) {
	return v1 + s * (v2 - v1) + t * (v3 - v1);
}

uint64_t createAxis(SystemInterface::Engine& engine, const std::string& path, const glm::vec3& position, const glm::quat& rotation) {
	uint64_t id = engine.createEntity();

	Transform& transform = *engine.addComponent<Transform>(id);

	transform.position = position;
	transform.rotation = rotation;
	transform.scale = { 0.01f, 0.01f, 0.05f };

	transform.localTranslate(Transform::localForward * 0.5f); // poke the arrow through the mesh

	Renderer& renderer = engine.system<Renderer>();

	// although programs and textures aren't loaded mulitple times in the renderer, meshes currently are, so for testing and speed I'm storing them as static
	static uint32_t program = renderer.loadProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl");
	static uint32_t mesh = renderer.loadMesh(path + "arrow.obj");
	static GLuint texture = renderer.loadTexture(path + "arrow.png");

	Model& model = *engine.addComponent<Model>(id);

	model.meshContextId = mesh;
	model.programContextId = program;
	model.textureBufferId = texture;

	return id;
}

int main(int argc, char** argv) {
	SystemInterface::Engine engine(1024 * 1024 * 128); // 128 KB

	Window::ConstructorInfo windowInfo;
	Renderer::ConstructorInfo rendererInfo;

	engine.registerSystem<Window>(engine, windowInfo);
	engine.registerSystem<Controller>(engine);
	engine.registerSystem<Renderer>(engine, rendererInfo);

	SYSFUNC_CALL(SystemInterface, initiate, engine)(std::vector<std::string>(argv, argv + argc));

	// Testing state, all temporary, super messy, litterally for learning/testing only
	{
		const std::string path = upperPath(replace('\\', '/', argv[0])) + "data/";

		Window& window = engine.system<Window>();
		Controller& controller = engine.system<Controller>();
		Renderer& renderer = engine.system<Renderer>();

		{
			// Window setup
			Window::WindowInfo windowConfig;

			window.openWindow(windowConfig);

			// Renderer setup
			Renderer::ShapeInfo shapeInfo;
			shapeInfo.verticalFov = 90;
			shapeInfo.zDepth = 100000;

			renderer.reshape(shapeInfo);
			renderer.defaultTexture(path + "checker.png");
			renderer.defaultProgram(path + "vertexShader.glsl", path + "fragmentShader.glsl");
		}

		// Camera
		{
			uint64_t id = engine.createEntity();

			Transform& transform = *engine.addComponent<Transform>(id);
			transform.position = { 0.f, -100.f, 100.f };
			transform.rotation = glm::quat({ glm::radians(90.f), 0.f, 0.f });

			renderer.setCamera(id);
			controller.setPossessed(id);
		}

		// Scene
		uint64_t sceneParent;

		{
			uint64_t id = engine.createEntity();
			
			Transform& transform = *engine.addComponent<Transform>(id);
			transform.rotation = glm::quat({ glm::radians(90.f) , 0.f, 0.f });
			//transform.scale = { 10.f, 10.f, 10.f };
		
			renderer.loadMesh(path + "triangle_test_crooked.fbx", id);

			sceneParent = id;
			
		}

		// Skybox
		{
			uint64_t id = engine.createEntity();
			
			Transform& transform = *engine.addComponent<Transform>(id);
			transform.scale = { 1000.f, 1000.f, 1000.f };
			
			renderer.loadMesh(path + "skybox.obj", id);
			recursivelySetTexture(engine, id, renderer.loadTexture(path + "skybox.png"));
		}

		// Reloading scene for testing barycentric interpolation, should be compute shader working with opengl buffers eventually
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(path + "triangle_test_crooked.fbx", aiProcessPreset_TargetRealtime_MaxQuality);
		const aiNode& node = *scene->mRootNode->FindNode("triangle");
		const aiMesh& mesh = *scene->mMeshes[node.mMeshes[0]];

		// find the testing triangle in the scene
		uint64_t triangleEntity = recursivelyFindName(engine, sceneParent, "triangle");

		Transform* triangleTransform = engine.getComponent<Transform>(triangleEntity);

		glm::mat4 triangleModelMatrix = triangleTransform->globalMatrix();

		glm::quat triangleRotation;
		glm::decompose(triangleModelMatrix, glm::vec3(), triangleRotation, glm::vec3(), glm::vec3(), glm::vec4());

		// http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
		// https://web.archive.org/web/20160311085440/http://freespace.virgin.net/hugo.elias/radiosity/radiosity.htm

		// Barycentric interpolate from UV texels to global positions
		struct Vertex {
			glm::vec3 position;
			glm::vec2 texcoord;
			glm::vec3 normal;
		};

		Vertex triangle[3];

		for (uint8_t i = 0; i < 3; i++)
			triangle[i] = { *(glm::vec3*)&mesh.mVertices[i], *(glm::vec2*)&mesh.mTextureCoords[0][i], *(glm::vec3*)&mesh.mNormals[i] };
		
		std::sort(triangle, triangle + 3, [](const Vertex& a, const Vertex& b) -> bool {
			return a.texcoord.y < b.texcoord.y;
		});
		
		glm::vec2 textureSize = { 64.f, 64.f };

		glm::vec2 vt1 = triangle[0].texcoord * textureSize;
		glm::vec2 vt2 = triangle[1].texcoord * textureSize;
		glm::vec2 vt3 = triangle[2].texcoord * textureSize;

		int minX = glm::floor(glm::min(vt1.x, glm::min(vt2.x, vt3.x)));
		int maxX = glm::ceil(glm::max(vt1.x, glm::max(vt2.x, vt3.x)));
		int minY = glm::floor(glm::min(vt1.y, glm::min(vt2.y, vt3.y)));
		int maxY = glm::ceil(glm::max(vt1.y, glm::max(vt2.y, vt3.y)));
		
		glm::vec2 vs1 = vt2 - vt1; // vt1 vertex to vt2
		glm::vec2 vs2 = vt3 - vt1; // vt1 vertex to vt3

		for (int x = minX; x <= maxX; x++) {
			for (int y = minY; y <= maxY; y++) {
				glm::vec2 q{ x - vt1.x, y - vt1.y };

				q += glm::vec2{ 0.5f, 0.5f };

				float area2 = crossProduct(vs1, vs2); // area * 2 of triangle

				float s = crossProduct(q, vs2) / area2;
				float t = crossProduct(vs1, q) / area2;

				glm::vec3 position = baryInterpolate(triangle[0].position, triangle[1].position, triangle[2].position, s, t);
				glm::vec3 normal = baryInterpolate(triangle[0].normal, triangle[1].normal, triangle[2].normal, s, t);

				glm::vec3 worldPosition = triangleModelMatrix * glm::vec4(position, 1);
				glm::quat worldRotation = glm::inverse(glm::lookAt(worldPosition, worldPosition + normal, Transform::localUp));

				if ((s >= 0) && (t >= 0) && (s + t <= 1))
					createAxis(engine, path, worldPosition, worldRotation);
			}
		}
	}

	// Main loop
	TimePoint timer;
	double dt = 0.0;

	while (engine.running()) {
		startTime(&timer);

		SYSFUNC_CALL(SystemInterface, update, engine)(dt);
		SYSFUNC_CALL(SystemInterface, lateUpdate, engine)(dt);

		dt = deltaTime(timer);
	}
	
	return 0;
}