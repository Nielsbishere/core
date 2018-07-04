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
#include <utils/random.h>

#include <input/keyboard.h>

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
///Fix nameless structs (oish_gen)
///Support FBO textures;	FBOTexture inherrits GraphicsObject. FBOTexture is std::vector<Texture*> and every frame has to be updated :(
///							First transition to rt write and then to shader read after it ended
///Abstract Model
///RenderTarget support textures too; so a RenderTarget could also just be a bunch of textures you render to
///Support indirect rendering
///Multiple descriptor sets
///Android update project in CMake
///Abstract AssetManager
///Allow arrays in registers and buffers
///Support runtime shader compilation
///Quaternions
///Support file reloading
///Model validation with pipeline
///Abstract Entity
///Figure out why Android is throwing an error when it's rotated (width and height are flipped the wrong way)

//Set up the interface

void MainInterface::initScene() {

	Log::println("Started main interface!");

	getInputManager().load("res/settings/input.json");

	shader = g.create(ShaderInfo("res/shaders/simple.oiSH"));
	g.use(shader);

	pipelineState = g.create(PipelineStateInfo());
	g.use(pipelineState);

	//Vec3f position
	//Vec2f uv
	f32 vert[] = {

		//Bottom
		-1, -1, 1, 0, 1,
		1, -1, 1, 1, 1,
		1, -1, -1, 1, 0,
		-1, -1, -1, 0, 0,

		//Top
		-1, 1, -1, 1, 1,
		1, 1, -1, 0, 1,
		1, 1, 1, 0, 0,
		-1, 1, 1, 1, 0,

		//Back
		-1, -1, -1, 1, 0,
		1, -1, -1, 0, 0,
		1, 1, -1, 0, 1,
		-1, 1, -1, 1, 1,

		//Front
		-1, 1, 1, 0, 1,
		1, 1, 1, 1, 1,
		1, -1, 1, 1, 0,
		-1, -1, 1, 0, 0,

		//Left
		-1, 1, -1, 0, 1,
		-1, 1, 1, 1, 1,
		-1, -1, 1, 1, 0,
		-1, -1, -1, 0, 0,

		//Right
		1, -1, -1, 1, 0,
		1, -1, 1, 1, 1,
		1, 1, 1, 0, 1,
		1, 1, -1, 0, 0

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

	camera = g.create(CameraInfo(45.f, Vec3(15, 15, 55), Vec4(0, 0, 0, 1), Vec3(0, 1, 0), 0.1f, 100.f));
	g.use(camera);

	for (u32 i = 0; i < totalObjects; ++i)
		objects[i].m = Matrixf::makeModel(Random::randomize<3>(0.f, 25.f), Vec3f(Random::randomize<2>(0.f, 360.f)), Vec3f(1.f));

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

	//Execute draw call
	cmdList->bind({ quadVbo });
	cmdList->bind({ quadIbo });
	cmdList->drawIndexed(36, totalObjects);

	//End fbo and cmdList
	cmdList->end(rt);
	cmdList->end();

}

void MainInterface::initSurface(){

	GraphicsInterface::initSurface();

	//Reconstruct pipeline

	if (pipeline != nullptr) {
		g.destroy(pipeline);
		pipeline = nullptr;
	}

	if (getParent()->getInfo().getSize() != Vec2u())
		pipeline = g.create(PipelineInfo(shader, pipelineState, g.getBackBuffer(), camera));

	//Reconstruct all VP affected objects

	camera->bind(g.getBackBuffer()->getSize());

	for (u32 i = 0; i < totalObjects; ++i)
		objects[i].mvp = { camera->getBoundProjection() * camera->getBoundView() * objects[i].m };

	shader->get<ShaderBuffer>("Objects")->set(Buffer::construct((u8*)objects, (u32) sizeof(objects)));
}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {
	Log::println(b.toString());
}

void MainInterface::load(String path){ Log::println("Loading"); }
void MainInterface::save(String path){ Log::println("Saving"); }

void MainInterface::update(flp dt) {
	WindowInterface::update(dt); 

	if (getParent()->getInputHandler().getKeyboard()->isPressed(Key::F11))
		getParent()->getInfo().toggleFullScreen();
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