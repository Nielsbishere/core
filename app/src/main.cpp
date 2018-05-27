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
#include <graphics/sampler.h>
#include <graphics/camera.h>

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
///Support instancing / indirect rendering
///Support FBO textures; FBOTexture inherrits GraphicsObject. FBOTexture is std::vector<Texture*> and every frame has to be updated :(
///Abstract AssetManager
///Abstract Model
///Abstract Entity
///Allow arrays in registers and buffers
///Fix Window security
///Shader class validation (->set(name, var))
///Pipeline validation
///Android update project in CMake
///Android resize
///RenderTarget support textures too; so a RenderTarget could also just be a bunch of textures you render to

//Set up the interface

void MainInterface::initScene() {

	Log::println("Started main interface!");

	getInputManager().load("res/settings/input.json");

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

	osomi = g.create(TextureInfo("res/textures/osomi.png"));
	g.use(osomi);

	sampler = g.create(SamplerInfo(SamplerMin::Linear, SamplerMag::Linear, SamplerWrapping::Repeat));
	g.use(sampler);

	shader->set("samp", sampler);
	shader->set("tex", osomi);

	pipeline = nullptr;

	camera = g.create(CameraInfo(45.f, Vec3(0, 0, 1.5), Vec4(0, 0, 0, 1), Vec3(0, 1, 0), 0.1f, 100.f));
	g.use(camera);

}

void MainInterface::renderScene(){

	RenderTarget *rt = g.getBackBuffer();

	//Update per execution shader buffer

	ShaderBuffer *perExecution = shader->get<ShaderBuffer>("PerExecution");

	perExecution->open();
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
	perObject->set("mvp", camera->getBoundProjection() * camera->getBoundView() * m);
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
		pipeline = g.create(PipelineInfo(shader, pipelineState, g.getBackBuffer(), camera));
}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {
	Log::println(b.toString());
}

void MainInterface::load(String path){ Log::println("Loading"); }
void MainInterface::save(String path){ Log::println("Saving"); }

void MainInterface::update(flp dt) {

	crot += Vec3(32, 16, 8) * dt;
	cpos += Vec3(0, -.05, -0.3) * dt;

	WindowInterface::update(dt); 
}

MainInterface::~MainInterface(){
	g.finish();
	g.destroy(camera);
	g.destroy(sampler);
	g.destroy(osomi);
	g.destroy(quadVbo);
	g.destroy(quadIbo);
	g.destroy(pipeline);
	g.destroy(pipelineState);
	g.destroy(shader);
	g.destroy(cmdList);
}