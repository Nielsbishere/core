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
#include <graphics/format/oirm.h>
#include <graphics/format/obj.h>
#include <graphics/shaderbuffer.h>
#include <graphics/mesh.h>
#include <graphics/drawlist.h>

#include <graphics/format/fbx.h>

#include <types/matrix.h>
#include <graphics/rendertarget.h>
#include <graphics/versionedtexture.h>
#include <graphics/sampler.h>
#include <graphics/camera.h>
#include <utils/random.h>

#include <input/keyboard.h>
#include <input/mouse.h>

#include <math/simplexnoise.h>

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

void genPlanet(f32 (*displace)(Vec3 loc)) {

	Timer t;

	u32 resolution = 42;

	u32 resolutio = resolution - 1;
	u32 vertices = 6 * resolution * resolution;
	u32 indices = resolutio * resolutio * 36;

	Vec3 norm[] = {
		{ 0, 1, 0 },
		{ 0, -1, 0 },
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0, 0, 1 },
		{ 0, 0, -1 }
	};

	Vec3 tang[] = {
		{ 1, 0, 0 },
		{ -1, 0, 0 },
		{ 0, 0, 1 },
		{ 0, 0, -1 },
		{ 0, 1, 0 },
		{ 0, -1, 0 }
	};

	Vec3 bit[] = {
		norm[0].cross(tang[0]),
		norm[1].cross(tang[1]),
		norm[2].cross(tang[2]),
		norm[3].cross(tang[3]),
		norm[4].cross(tang[4]),
		norm[5].cross(tang[5])
	};

	std::vector<f32> vertex(vertices * 8);
	std::vector<u32> index(indices);
	f32 *avertex = vertex.data();
	u32 *aindex = index.data();

	for (u32 i = 0; i < vertices; ++i) {

		u32 x = i % resolution;
		u32 y = (i / resolution) % resolution;
		u32 z = i / resolution / resolution;

		Vec3 &up = norm[z];
		Vec3 &tangent = tang[z];
		Vec3 &bitangent = bit[z];

		Vec2 uv = Vec2((f32)x, (f32)y) / Vec2((f32)resolutio);
		Vec2 xy = uv * 2.f - 1.f;

		Vec3 pos = (up + tangent * xy.x + bitangent * xy.y).normalize();

		*(Vec3*)(avertex + 8 * i) = pos * (1 + displace(pos));
		*(Vec2*)(avertex + 8 * i + 3) = uv;		//Temporary planar projection
		*(Vec3*)(avertex + 8 * i + 5) = pos;	//Normal = position for a sphere; TODO: Use displacement for this

		if (x != resolutio && y != resolutio) {

			u32 ind = (z * resolutio * resolutio + y * resolutio + x) * 6;

			aindex[ind] = i;
			aindex[ind + 1] = i + resolution + 1;
			aindex[ind + 2] = i + resolution;
			aindex[ind + 3] = i;
			aindex[ind + 4] = i + 1;
			aindex[ind + 5] = i + resolution + 1;

		}

	}

	Buffer buf = oiRM::generate(Buffer::construct((u8*) avertex, vertices * 32), Buffer::construct((u8*) aindex, indices * 4), true, true, true, vertices, indices, true);

	FileManager::get()->write("out/models/planet.oiRM", buf);

	buf.deconstruct();

	t.stop();
	t.print();

}

float myNoise(Vec3 noise) {
	return pow(SimplexNoise::noise(noise * ((SimplexNoise::noise(noise * 1.5f + 399) * 0.5f + 0.5f) * 20.f + 3.f), 4, 1.5f, 0.5f), 1.5f) * 0.8f + 0.1f;
}

void MainInterface::initScene() {

	//
	genPlanet(myNoise);

	Log::println("Started main interface!");

	//Setup our input manager
	getInputManager().load("res/settings/input.json");

	//Setup our shader
	shader = g.create("Simple", ShaderInfo("res/shaders/simple.oiSH"));
	g.use(shader);

	//Setup our post process shader
	shader0 = g.create("Post process", ShaderInfo("res/shaders/post_process.oiSH"));
	g.use(shader0);

	//Setup our pipeline state (with default settings)
	pipelineState = g.create("Default pipeline state", PipelineStateInfo());
	g.use(pipelineState);

	//Setup our cube & sphere
	RMFile file;
	oiRM::read("res/models/anvil0.oiRM", file);
	auto info = oiRM::convert(&g, file);

	info.first.maxIndices = 300000;
	info.first.maxVertices = 200000;
	meshBuffer = g.create("Mesh buffer", info.first);
	g.use(meshBuffer);
	meshBuffer->open();

	info.second.buffer = meshBuffer;
	mesh = g.create("Cube", info.second);
	g.use(mesh);

	oiRM::read("res/models/sword.oiRM", file);
	info = oiRM::convert(&g, file);

	info.second.buffer = meshBuffer;
	mesh0 = g.create("Sphere", info.second);
	g.use(mesh0);

	meshBuffer->close();

	//Setup our quad
	oiRM::read("res/models/post_processing_quad.oiRM", file);
	info = oiRM::convert(&g, file);

	meshBuffer0 = g.create("Mesh buffer 1", info.first);
	g.use(meshBuffer0);
	meshBuffer0->open();

	info.second.buffer = meshBuffer0;
	mesh1 = g.create("Quad", info.second);
	g.use(mesh1);

	meshBuffer0->close();

	////Setup our drawList (indirect)
	//drawList = g.create("Draw list", DrawListInfo(meshBuffer, shader->get<ShaderBuffer>("Objects")->getBuffer(), 256, false));
	//g.use(drawList);

	//Allocate a texture
	osomi = g.create("osomi", TextureInfo("res/textures/osomi.png"));
	g.use(osomi);

	//Allocate sampler
	sampler = g.create("Default sampler", SamplerInfo(SamplerMin::Linear, SamplerMag::Linear, SamplerWrapping::Repeat));
	g.use(sampler);

	//Set our shader sampler and texture
	shader->set("samp", sampler);
	shader->set("tex", osomi);

	//Setup our post-process sampler
	shader0->set("samp", sampler);

	//Setup our camera
	camera = g.create("Default camera", CameraInfo(45.f, Vec3(5, 5, 5), Vec4(0, 0, 0, 1)));
	g.use(camera);

	////Setup our objects (indirect)
	//for (u32 i = 0; i < totalObjects; ++i)
	//	objects[i].m = Matrix::makeModel(Random::randomize<3>(0.f, 12.f), Vec3(Random::randomize<2>(0.f, 360.f)), Vec3(1.f));

}

void MainInterface::renderScene(){

	//Update per execution shader buffer
	ShaderBuffer *perExecution = shader->get<ShaderBuffer>("PerExecution");

	perExecution->open();
	perExecution->set("ambient", Vec3(1));
	perExecution->set("time", (f32) getRuntime());
	perExecution->set("power", 1.f);
	perExecution->close();

	//Setup post processing settings
	ShaderBuffer *postProcessing = shader0->get<ShaderBuffer>("PostProcessingSettings");

	postProcessing->open();
	postProcessing->set("exposure", exposure);
	postProcessing->set("gamma", gamma);
	postProcessing->close();

	//Setup lighting
	ShaderBuffer *directionalLight = shader->get<ShaderBuffer>("DirectionalLights");

	directionalLight->open();
	directionalLight->set("dir", Vec3(-1, -1, -1));
	directionalLight->set("intensity", 2.f);
	directionalLight->set("col", Vec3(1.f));
	directionalLight->close();

	//Start 'rendering'
	cmdList->begin();

	//Render to renderTarget

		//Bind fbo and pipeline
		cmdList->begin(renderTarget, Vec4d(0.25, 0.5, 1, 1) * (sin(getDuration()) * 0.5 + 0.5));
		cmdList->bind(pipeline);

		//Execute indirect draw calls
		//cmdList->draw(drawList);
		
		cmdList->draw(mesh);

		//End rt
		cmdList->end(renderTarget);

	//Render to backbuffer

		//Execute our post processing shader
		cmdList->begin(g.getBackBuffer());
		cmdList->bind(pipeline0);
		cmdList->draw(mesh1);
		cmdList->end(g.getBackBuffer());

	cmdList->end();

}

void MainInterface::initSceneSurface(){

	//Reconstruct pipeline

	if (pipeline != nullptr) {

		g.destroy(renderTarget);
		renderTarget = nullptr;

		g.destroy(pipeline);
		pipeline = nullptr;

		g.destroy(pipeline0);
		pipeline = nullptr;

	}

	Vec2u res = g.getBackBuffer()->getSize();

	if (res.x == 0 || res.y == 0)
		return;

	renderTarget = g.create("Post processing target", RenderTargetInfo(res, TextureFormat::Depth, { TextureFormat::RGBA16f }));
	g.use(renderTarget);

	shader0->set("tex", renderTarget->getTarget(0));

	pipeline = g.create("Rendering pipeline", PipelineInfo(shader, pipelineState, renderTarget, meshBuffer, camera));
	g.use(pipeline);

	pipeline0 = g.create("Post process pipeline", PipelineInfo(shader0, pipelineState, g.getBackBuffer(), meshBuffer0, camera));
	g.use(pipeline0);

	////Setup indirect draws

	//Reconstruct all VP affected objects
	//camera->bind(res);

	/*for (u32 i = 0; i < totalObjects; ++i)
		objects[i].mvp = { camera->getBoundProjection() * camera->getBoundView() * objects[i].m };*/

	//drawList->clear();
	//drawList->draw(mesh, totalObjects / 2, Buffer::construct((u8*)objects, (u32) sizeof(objects) / 2));
	//drawList->draw(mesh0, totalObjects / 2, Buffer::construct((u8*)objects + sizeof(objects) / 2, (u32) sizeof(objects) / 2));
	//drawList->flush();

}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {
	Log::println(b.toString());
}

void MainInterface::update(f32 dt) {

	WindowInterface::update(dt); 

	if (getParent()->getInputHandler().getKeyboard()->isPressed(Key::F11))
		getParent()->getInfo().toggleFullScreen();

	Vec2 nextMouse;
	nextMouse.x = getParent()->getInputHandler().getMouse()->getAxis(MouseAxis::X);
	nextMouse.y = getParent()->getInputHandler().getMouse()->getAxis(MouseAxis::Y);

	if (getParent()->getInputHandler().getMouse()->isDown(MouseButton::Left)) {

		Vec2 delta = nextMouse - prevMouse;

		exposure += delta.x / 3;
		gamma += delta.y / 3;

		Log::println(String(gamma) + ", " + exposure);

	}

	prevMouse = nextMouse;
	planetRotation += Vec3(30, 50) * dt;

	//Update planet rotation

	camera->bind(getParent()->getInfo().getSize());
	objects[0].m = Matrix::makeModel(Vec3(), Vec3(planetRotation, 0.f), Vec3(1.f));
	objects[0].mvp = { camera->getBoundProjection() * camera->getBoundView() * objects[0].m };

	ShaderBuffer *objects = shader->get<ShaderBuffer>("Objects");
	objects->set(Buffer::construct((u8*)this->objects, (u32)sizeof(this->objects)));

}

MainInterface::~MainInterface(){
	g.finish();
	g.destroy(camera);
	g.destroy(sampler);
	g.destroy(osomi);
	g.destroy(mesh);
	g.destroy(mesh0);
	g.destroy(mesh1);
	g.destroy(meshBuffer);
	g.destroy(meshBuffer0);
	//g.destroy(drawList);
	g.destroy(renderTarget);
	g.destroy(pipeline);
	g.destroy(pipeline0);
	g.destroy(pipelineState);
	g.destroy(shader);
	g.destroy(shader0);
	g.destroy(cmdList);
}