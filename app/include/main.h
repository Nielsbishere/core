#pragma once

#include <graphics/graphicsinterface.h>
#include <types/vector.h>
#include <types/matrix.h>
#include <platforms/generic.h>
#include <graphics/texture.h>

struct Application {
	static void instantiate(oi::wc::WindowHandleExt *param = nullptr);
};

struct MeshFormatEx {

	oi::Vec3f position;
	oi::Vec2f uv;

	static MeshFormatEx cubeVertices[];
	static u32 cubeIndices[];

	static MeshFormatEx pyramidVertices[];
	static u32 pyramidIndices[];

	static std::vector<std::vector<oi::gc::TextureFormat>> vertexData;

};

class MainInterface : public oi::gc::GraphicsInterface {

public:

	~MainInterface();
	void initScene() override;
	void renderScene() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;
	
	void load(oi::String path) override {}
	void save(oi::String path) override {}

	void update(flp dt) override;
	void initSurface() override;

private:
	
	oi::gc::Shader *shader, *shader0;
	oi::gc::Pipeline *pipeline, *pipeline0;
	oi::gc::PipelineState *pipelineState;
	oi::gc::Texture *osomi;
	oi::gc::Sampler *sampler;
	oi::gc::Camera *camera;
	oi::gc::MeshBuffer *meshBuffer;
	oi::gc::Mesh *mesh, *mesh0;
	oi::gc::DrawList *drawList;
	oi::gc::RenderTarget *renderTarget;
	oi::gc::GBuffer *quadVbo;

	float exposure = 5, gamma = 2.2f;
	oi::Vec2f prevMouse;

	struct PerObject {

		oi::Matrixf m;
		oi::Matrixf mvp;

	};

	static constexpr u32 totalObjects = 64U;

	PerObject objects[totalObjects];

};