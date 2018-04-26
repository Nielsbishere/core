#include "main.h"
#include <window/windowmanager.h>
#include <file/filemanager.h>

#include <graphics/commandlist.h>
#include <graphics/shader.h>
#include <graphics/pipeline.h>
#include <graphics/pipelinestate.h>
#include <graphics/gbuffer.h>

#include <graphics/format/oish.h>

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

//Set up a main window
void Application::instantiate(void *param){
	FileManager fmanager(param);
	WindowManager wmanager;
	Window *w = wmanager.create(WindowInfo(__PROJECT_NAME__, 1, param));
	w->setInterface(new MainInterface());
	wmanager.waitAll();
}

//Set up the interface

void MainInterface::initScene(){

	Log::println("Started main interface!");

	getInputManager().load("res/settings/input.json");

	cmdList = g.create(CommandListInfo());
	g.use(cmdList);

	shader = g.create(ShaderInfo("res/shaders/simple"));
	g.use(shader);

	pipelineState = g.create(PipelineStateInfo());
	g.use(pipelineState);


	f32 vert[] = {
		-0.5, -0.5f, 1, 1, 1,
		-0.5f, 0.5f, 0, 1, 0,
		0.8f, 0.5f, 0, 0, 1,
		0.5f, -0.5f, 1, 0, 0
	};

	u32 ind[] = {
		0, 1, 2,
		2, 3, 0
	};

	quadVbo = g.create(GBufferInfo(GBufferType::VBO, (u32) sizeof(vert), (u8*) vert));
	g.use(quadVbo);

	quadIbo = g.create(GBufferInfo(GBufferType::IBO, (u32) sizeof(ind), (u8*) ind));
	g.use(quadIbo);

	pipeline = nullptr;

}

void MainInterface::renderScene(){
	cmdList->begin();
	cmdList->begin(g.getBackBuffer(), Vec4d(0.25, 0.5, 1, 1) * (sin(getDuration()) * 0.5 + 0.5));
	cmdList->bind(pipeline);
	cmdList->bind({ quadVbo });
	cmdList->bind({ quadIbo });
	cmdList->drawIndexed(6);
	cmdList->end(g.getBackBuffer());
	cmdList->end();
}

void MainInterface::initSurface(){

	GraphicsInterface::initSurface();

	g.destroy(pipeline);
	pipeline = g.create(PipelineInfo(shader, pipelineState, g.getBackBuffer()));
}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {
	Log::println(b.toString());
}

void MainInterface::load(String path){ Log::println("Loading"); }
void MainInterface::save(String path){ Log::println("Saving"); }

void MainInterface::update(flp dt){  }

MainInterface::~MainInterface(){
	g.finish();
	g.destroy(quadVbo);
	g.destroy(quadIbo);
	g.destroy(pipeline);
	g.destroy(pipelineState);
	g.destroy(shader);
	g.destroy(cmdList);
}