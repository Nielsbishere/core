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

#include <types/matrix.h>
#include <graphics/rendertarget.h>

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

///TODO:
///Abstract Camera
///Texture and Sampler
///Abstract Entity
///Abstract Model
///Abstract AssetManager

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

	//Vec3f position, color
	//Vec2f uv
	f32 vert[] = {

		//Bottom
		-1, -1, 1, 0, 0, 1, 0, 1,
		1, -1, 1, 1, 0, 1, 1, 1,
		1, -1, -1, 1, 0, 0, 1, 0,
		-1, -1, -1, 0, 0, 0, 0, 0,

		//Top
		-1, 1, -1, 0, 1, 0, 1, 1,
		1, 1, -1, 1, 1, 0, 0, 1,
		1, 1, 1, 1, 1, 1, 0, 0,
		-1, 1, 1, 0, 1, 1, 1, 0,

		//Back
		-1, -1, -1, 0, 0, 0, 1, 0,
		1, -1, -1, 1, 0, 0, 0, 0,
		1, 1, -1, 1, 1, 0, 0, 1,
		-1, 1, -1, 0, 1, 0, 1, 1,

		//Front
		-1, 1, 1, 0, 1, 1, 0, 1,
		1, 1, 1, 1, 1, 1, 1, 1,
		1, -1, 1, 1, 0, 1, 1, 0,
		-1, -1, 1, 0, 0, 1, 0, 0,

		//Left
		-1, 1, -1, 0, 1, 0, 0, 1,
		-1, 1, 1, 0, 1, 1, 1, 1,
		-1, -1, 1, 0, 0, 1, 1, 0,
		-1, -1, -1, 0, 0, 0, 0, 0,

		//Right
		1, -1, -1, 1, 0, 0, 1, 0,
		1, -1, 1, 1, 0, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 0, 1,
		1, 1, -1, 1, 1, 0, 0, 0

	};

	u32 ind[] = {

		//Bottom
		0, 1, 2,
		2, 3, 0,

		//Top
		4, 5, 6,
		6, 7, 4,

		//Back
		8, 9, 10,
		10, 11, 8,

		//Front
		12, 13, 14,
		14, 15, 12,

		//Left
		16, 17, 18,
		18, 19, 16,

		//Right
		20, 21, 22,
		22, 23, 20

	};

	quadVbo = g.create(GBufferInfo(GBufferType::VBO, (u32) sizeof(vert), (u8*)vert));
	g.use(quadVbo);

	quadIbo = g.create(GBufferInfo(GBufferType::IBO, (u32) sizeof(ind), (u8*)ind));
	g.use(quadIbo);

	pipeline = nullptr;

}

void MainInterface::renderScene(){

	const f32 fov = 45.f, near = 0.1f, far = 100.f;
	const Vec3 eye = { 0, 5, 10 }, center = {}, up = { 0, 1, 0 };

	RenderTarget *rt = g.getBackBuffer();

	//Update per execution shader buffer

	ShaderBuffer *perExecution = shader->get<ShaderBuffer>("PerExecution");

	Matrixf p = Matrixf::makePerspective(fov, Vec2(rt->getSize()).getAspect(), near, far);
	Matrixf v = Matrixf::makeView(eye, center, up);

	perExecution->open();
	perExecution->set("p", p);
	perExecution->set("v", v);
	perExecution->set("ambient", Vec3f(1));
	perExecution->set("time", (f32) getRuntime());
	perExecution->close();

	//Bind fbo and pipeline
	cmdList->begin();
	cmdList->begin(rt, Vec4d(0.25, 0.5, 1, 1) * (sin(getDuration()) * 0.5 + 0.5));
	cmdList->bind(pipeline);

	//Setup draw call
	
	ShaderBuffer *perObject = shader->get<ShaderBuffer>("PerObject");

	Matrixf m = Matrixf::makeModel(cpos, crot, cscale);

	perObject->open();
	perObject->set("m", m);
	perObject->set("mvp", p * v * m);
	perObject->close();

	//Execute draw call
	cmdList->bind({ quadVbo });
	cmdList->bind({ quadIbo });
	cmdList->drawIndexed(36);

	//End fbo and cmdList
	cmdList->end(rt);
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

void MainInterface::update(flp dt) {

	crot += Vec3(32, 16, 8) * dt;

	WindowInterface::update(dt); 
}

MainInterface::~MainInterface(){
	g.finish();
	g.destroy(quadVbo);
	g.destroy(quadIbo);
	g.destroy(pipeline);
	g.destroy(pipelineState);
	g.destroy(shader);
	g.destroy(cmdList);
}