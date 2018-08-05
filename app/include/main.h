#pragma once

#include <graphics/graphicsinterface.h>
#include <types/vector.h>
#include <types/matrix.h>
#include <platforms/generic.h>
#include <graphics/texture.h>

struct Application {
	static void instantiate(oi::wc::WindowHandleExt *param = nullptr);
};

class MainInterface : public oi::gc::GraphicsInterface {

public:

	~MainInterface();
	void initScene() override;
	void renderScene() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;
	
	void load(oi::String path) override {}
	void save(oi::String path) override {}

	void update(f32 dt) override;
	void initSceneSurface() override;

private:
	
	oi::gc::Shader *shader, *shader0;
	oi::gc::Pipeline *pipeline, *pipeline0;
	oi::gc::PipelineState *pipelineState;
	oi::gc::Texture *osomi;
	oi::gc::Sampler *sampler;
	oi::gc::Camera *camera;
	oi::gc::MeshBuffer *meshBuffer, *meshBuffer0;
	oi::gc::Mesh *mesh, *mesh0, *mesh1;
	oi::gc::DrawList *drawList;
	oi::gc::RenderTarget *renderTarget;

	float exposure = .15f, gamma = .85f;
	oi::Vec2 prevMouse;

	struct PerObject {

		oi::Matrix m;
		oi::Matrix mvp;

	};

	static constexpr u32 totalObjects = 64U;

	PerObject objects[totalObjects];

};