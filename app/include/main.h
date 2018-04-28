#pragma once

#include <graphics/graphicsinterface.h>
#include <types/vector.h>
#include <platforms/generic.h>

struct Application {
	static void instantiate(oi::wc::WindowHandleExt *param = nullptr);
};

class MainInterface : public oi::gc::GraphicsInterface {

public:

	~MainInterface();
	void initScene() override;
	void renderScene() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;
	
	void load(oi::String str) override;
	void save(oi::String str) override;
	void update(flp dt) override;
	void initSurface() override;

private:

	oi::gc::CommandList *cmdList;
	oi::gc::Shader *shader;
	oi::gc::Pipeline *pipeline;
	oi::gc::PipelineState *pipelineState;
	oi::gc::GBuffer *quadVbo, *quadIbo;

};