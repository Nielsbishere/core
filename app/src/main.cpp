#include "main.h"
#include <window/windowmanager.h>
#include <file/filemanager.h>

#include <graphics/commandlist.h>
#include <graphics/shader.h>
#include <graphics/pipeline.h>
#include <graphics/pipelinestate.h>
#include <graphics/gbuffer.h>

#include <graphics/format/oisb.h>
#include <format/oisl.h>
#include <graphics/shaderbuffer.h>

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

//Set up a main window
void Application::instantiate(WindowHandleExt *param){
	FileManager fmanager(param);
	WindowManager wmanager;
	Window *w = wmanager.create(WindowInfo(__PROJECT_NAME__, 1, param));
	w->setInterface(new MainInterface());
	wmanager.waitAll();
}

//Set up the interface

void MainInterface::initScene() {

	Log::println("Started main interface!");

	getInputManager().load("res/settings/input.json");

	cmdList = g.create(CommandListInfo());
	g.use(cmdList);

	shader = g.create(ShaderInfo("res/shaders/simple.oiSH"));
	g.use(shader);

	pipelineState = g.create(PipelineStateInfo());
	g.use(pipelineState);

	f32 vert[] = {

		-1, -1, -1, 1, 1, 1,
		1, -1, -1, 0, 1, 0,
		-1, 1, -1, 0, 0, 1,
		1, 1, -1, 1, 0, 0,

		-1, -1, 1, 0, 1, 1,
		1, -1, 1, 1, 1, 0,
		-1, 1, 1, 1, 0, 1,
		1, 1, 1, 0.5f, 0, 1

	};

	u32 ind[] = {

		//Back
		2, 1, 0,
		2, 3, 1,

		//Front
		6, 5, 4,
		6, 7, 5,

		//Left
		6, 2, 0,
		4, 6, 0,

		//Right
		1, 3, 5,
		3, 7, 5,

		////Bottom
		0, 1, 4,
		1, 5, 4,

		////Top
		2, 3, 6,
		3, 7, 6

	};

	quadVbo = g.create(GBufferInfo(GBufferType::VBO, (u32) sizeof(vert), (u8*)vert));
	g.use(quadVbo);

	quadIbo = g.create(GBufferInfo(GBufferType::IBO, (u32) sizeof(ind), (u8*)ind));
	g.use(quadIbo);

	pipeline = nullptr;

	///Temporary (since matrix math isn't added yet)

	//Object transform:
	//position: 0,0,0
	//rotation: 45,30,15
	//scale: 1.5, 1.25, 1
	f32 perObject[] = {

		//m
		1.25477445f, 0.786778331f, -0.237740576f, 0,
		-0.280179799f, 0.739383101f, 0.968148589f, 0,
		0.499999911f, -0.612372339f, 0.612372577f, 0,
		0, 0, 0, 1,

		//mvp
		1.70397794f, 1.95560229f, -0.139495030f, -0.139216334f,
		-0.380482882f, 0.551298261f, -1.19899619f, -1.19660056f,
		0.678997576f, -1.98347938f, -0.274409741f, -0.273861468f,
		0, 0, 11.0025225f, 11.1803398f

	};

	shader->get<ShaderBuffer>("PerObject")->set(Buffer::construct((u8*)perObject, (u32) sizeof(perObject)));

	//Camera transform:
	//lookat: 0,0,0
	//eye: 0,5,10
	//up: 0,1,0
	//fov: 45
	//aspect: 16.f / 9
	//near: 0.1f
	//far: 100
	f32 perExecution[] = {

		//p
		1.35799539f, 0, 0, 0,
		0, 2.41421413f, 0, 0,
		0, 0, -1.00200200f, -1,
		0, 0, -0.200200200f, 0,

		//v
		1, 0, 0, 0,
		0, 0.894427240f, 0.447213590f, 0,
		0, -0.447213620f, 0.894427180f, 0,
		0, 0, -11.1803398f, 1,

		//ambient
		0.25f, 0.5f, 1,

		//time
		0

	};

	shader->get<ShaderBuffer>("PerExecution")->set(Buffer::construct((u8*)perExecution, (u32) sizeof(perExecution)));
}

void MainInterface::renderScene(){
	cmdList->begin();
	cmdList->begin(g.getBackBuffer(), Vec4d(0.25, 0.5, 1, 1) * (sin(getDuration()) * 0.5 + 0.5));
	cmdList->bind(pipeline);
	cmdList->bind({ quadVbo });
	cmdList->bind({ quadIbo });
	cmdList->drawIndexed(36);
	cmdList->end(g.getBackBuffer());
	cmdList->end();
}

void MainInterface::initSurface(){

	GraphicsInterface::initSurface();

	if (pipeline != nullptr) {
		g.destroy(pipeline);
		pipeline = nullptr;
	}

	if (getParent()->getInfo().getSize() != Vec2u())
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