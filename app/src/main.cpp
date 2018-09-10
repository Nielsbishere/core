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
#include <graphics/texturelist.h>
#include <graphics/sampler.h>
#include <graphics/camera.h>
#include <utils/random.h>
#include <utils/json.h>

#include <input/keyboard.h>
#include <input/mouse.h>

#include <math/simplexnoise.h>

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

//Set up a main window
void Application::instantiate(WindowHandleExt *param){
	Random::seedRandom();
	FileManager fmanager(param);
	WindowManager wmanager;
	Window *w = wmanager.create(WindowInfo(__PROJECT_NAME__, 1, param));
	w->setInterface(new MainInterface());
	wmanager.waitAll();
}

//Set up the interface

f32 NoiseLayer::sample(Vec3 pos, f32 mask) {
	f32 value = SimplexNoise::noise(pos, offset, octaves, persistence, roughness, frequency);
	return value > minValue ? (value - minValue) * scale * (maskLand ? mask : 1) : 0;
}

f32 Planet::sample(Vec3 pos) {

	f32 val = minHeight;

	for (u32 i = 0; i < (u32)noiseLayer.size(); ++i)
		if (noiseLayer[i].enabled)
			val += noiseLayer[i].sample(pos + offset, std::clamp((val + 0.5f) / coastSize, 0.f, 1.f));

	return val * scale;

}

void Planet::seed() {
	if (randomize)
		offset = Random::randomize<3>(-50000.f, 50000.f);
}

void MainInterface::refreshPlanet(Planet planet) {

	Timer t;

	u32 resolution = 40;

	u32 resolutio = resolution - 1;
	u32 vertices = 6 * resolution * resolution;
	u32 indices = resolutio * resolutio * 36;

	Vec3 norm[] = {
		{ 0, 1, 0 },		//0 = up
		{ 1, 0, 0 },		//1 = right
		{ 0, -1, 0 },		//2 = down
		{ -1, 0, 0 },		//3 = left
		{ 0, 0, 1 },		//4 = front
		{ 0, 0, -1 }		//5 = back
	};

	Vec3 tang[] = {
		{ 1, 0, 0 },
		{ 0, 0, 1 },
		{ -1, 0, 0 },
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

	planet.seed();

	for (u32 i = 0; i < vertices; ++i) {

		u32 x = i % resolution;
		u32 y = (i / resolution) % resolution;
		u32 z = i / resolution / resolution;

		Vec3 &up = norm[z];
		Vec3 &tangent = tang[z];
		Vec3 &bitangent = bit[z];

		Vec2 uv = Vec2((f32)x, (f32)y) / Vec2((f32)resolutio);
		Vec2 xy = uv * 2.f - 1.f;

		Vec3 cpos = (up + tangent * xy.x + bitangent * xy.y);
		Vec3 spos = cpos.normalize();

		*(Vec3*)(avertex + 8 * i) = spos * (1 + planet.sample(spos));
		*(Vec2*)(avertex + 8 * i + 3) = uv;		//Temporary planar projection
		*(Vec3*)(avertex + 8 * i + 5) = spos;	//Normal = position for a sphere; TODO: Use displacement for this

		if (x != resolutio && y != resolutio) {

			u32 ind = (z * resolutio * resolutio + y * resolutio + x) * 6;

			aindex[ind] = i + resolution;
			aindex[ind + 1] = i + 1;
			aindex[ind + 2] = i;
			aindex[ind + 3] = i + resolution;
			aindex[ind + 4] = i + resolution + 1;
			aindex[ind + 5] = i + 1;

		}

	}

	//TODO: Load into MeshInfo directly

	Buffer buf = oiRM::generate(Buffer::construct((u8*) avertex, vertices * 32), Buffer::construct((u8*) aindex, indices * 4), true, true, true, vertices, indices, false);

	FileManager::get()->write("out/models/planet.oiRM", buf);

	buf.deconstruct();

	if (mesh3 != nullptr)
		g.destroy(mesh3);

	meshBuffer->open();

	RMFile file;
	oiRM::read("out/models/planet.oiRM", file);
	auto info = oiRM::convert(file);

	info.second.buffer = meshBuffer;
	mesh3 = g.create("planet", info.second);
	g.use(mesh3);

	meshBuffer->close();

	t.stop();
	t.print();

}

void MainInterface::writePlanets() {

	JSON json;
	json.serialize(planets, true);
	FileManager::get()->write("out/models/planets.json", json.toString());

	RMFile file = oiRM::convert(mesh3->getInfo());
	oiRM::write(file, String("out/models/") + mesh3->getName() + ".oiRM", false);
}

void MainInterface::readPlanets(bool fromResource) {

	String str;
	FileManager::get()->read(String(fromResource ? "res/models/planets.json" : "out/models/planets.json"), str);
	JSON json = str;
	json.serialize(planets, false);

	for(auto &elem : planets)
		refreshPlanet(elem.second);

}

void MainInterface::initScene() {

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
	PipelineStateInfo psi;
	psi.lineWidth = 3.f;
	pipelineState = g.create("Default pipeline state", psi);
	g.use(pipelineState);

	//Setup our cube & sphere
	RMFile file;
	oiRM::read("res/models/anvil.oiRM", file);
	auto info = oiRM::convert(file);

	info.first.maxIndices = 300000;
	info.first.maxVertices = 200000;
	info.first.topologyMode = TopologyMode::Triangle;
	info.first.fillMode = FillMode::Fill;
	meshBuffer = g.create("Mesh buffer", info.first);
	g.use(meshBuffer);
	meshBuffer->open();

	info.second.buffer = meshBuffer;
	mesh = g.create("Anvil", info.second);
	g.use(mesh);

	oiRM::read("res/models/sword.oiRM", file);
	info = oiRM::convert(file);

	info.second.buffer = meshBuffer;
	mesh0 = g.create("Sword", info.second);
	g.use(mesh0);

	oiRM::read("res/models/sphere.oiRM", file);
	info = oiRM::convert(file);

	info.second.buffer = meshBuffer;
	mesh2 = g.create("Sphere", info.second);
	g.use(mesh2);

	meshBuffer->close();

	readPlanets(true);

	//Setup our quad
	oiRM::read("res/models/post_processing_quad.oiRM", file);
	info = oiRM::convert(file);
	info.first.topologyMode = TopologyMode::Triangle;
	info.first.fillMode = FillMode::Fill;

	meshBuffer0 = g.create("Mesh buffer 1", info.first);
	g.use(meshBuffer0);
	meshBuffer0->open();

	info.second.buffer = meshBuffer0;
	mesh1 = g.create("Quad", info.second);
	g.use(mesh1);

	meshBuffer0->close();

	//Setup our drawLists (indirect)
	drawList = g.create("Draw list (main geometry)", DrawListInfo(meshBuffer, 256, false));
	g.use(drawList);

	drawList0 = g.create("Draw list (second pass)", DrawListInfo(meshBuffer0, 1, false));
	g.use(drawList0);

	drawList0->draw(mesh1, 1);
	drawList0->flush();

	//Allocate textures
	osomi = g.create("osomi", TextureInfo("res/textures/osomi.png"));
	g.use(osomi);

	rock = g.create("rock", TextureInfo("res/textures/rock_dif.png"));
	g.use(rock);

	water = g.create("water", TextureInfo("res/textures/water_dif.png"));
	g.use(water);

	TextureList *tex = shader->get<TextureList>("tex");
	hrock = tex->alloc(rock);
	hwater = tex->alloc(water);

	//Allocate sampler
	sampler = g.create("Default sampler", SamplerInfo(SamplerMin::Linear, SamplerMag::Linear, SamplerWrapping::Repeat));
	g.use(sampler);

	//Set our shader sampler
	shader->set("samp", sampler);

	//Setup our post-process sampler
	shader0->set("samp", sampler);

	//Setup our camera
	camera = g.create("Default camera", CameraInfo(45.f, Vec3(3, 3, 3), Vec4(0, 0, 0, 1)));
	g.use(camera);

	//Setup lighting
	shader->get<ShaderBuffer>("PointLights")->instantiate(1);
	shader->get<ShaderBuffer>("SpotLights")->instantiate(1);

	ShaderBuffer *directionalLights = shader->get<ShaderBuffer>("DirectionalLights")->instantiate(1);

	directionalLights->open();
	directionalLights->set("light/dir", Vec3(0, -1, 0));
	directionalLights->set("light/intensity", 16.f);
	directionalLights->set("light/col", Vec3(1.f));
	directionalLights->close();

	//Setup the Objects buffer with our size
	shader->get<ShaderBuffer>("Objects")->instantiate(totalObjects);

	//Setup drawcalls  (reserve objects for meshes)

	drawList->draw(mesh2, 1);		//Reserve index 0 for the sphere
	drawList->draw(mesh3, 1);		//Reserve index 1 for the planet
	drawList->flush();

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

	//Start 'rendering'
	cmdList->begin();

	//Render to renderTarget

		//Bind fbo and pipeline
		cmdList->begin(renderTarget);
		cmdList->bind(pipeline);
		cmdList->draw(drawList);
		cmdList->end(renderTarget);

	//Render to backbuffer

		//Execute our post processing shader
		cmdList->begin(g.getBackBuffer());
		cmdList->bind(pipeline0);
		cmdList->draw(drawList0);
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

}
	
void MainInterface::onInput(InputDevice *device, Binding b, bool down) {

	Log::println(b.toString());

	if (b.getBindingType() == BindingType::KEYBOARD) {

		if (b.toKey() == Key::Volume_up || b.toKey() == Key::Up)
			writePlanets();
		else if (b.toKey() == Key::Volume_down || b.toKey() == Key::Down)
			readPlanets(true);

	}

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

	objects[0].m = Matrix::makeModel(Vec3(), Vec3(planetRotation, 0.f), Vec3(1.5f));
	objects[0].mvp = { camera->getBoundProjection() * camera->getBoundView() * objects[0].m };
	objects[0].diffuse = hwater;

	objects[1].m = Matrix::makeModel(Vec3(), Vec3(planetRotation, 0.f), Vec3(3.f));
	objects[1].mvp = { camera->getBoundProjection() * camera->getBoundView() * objects[1].m };
	objects[1].diffuse = hrock;

	shader->get<ShaderBuffer>("Objects")->getBuffer()->set(Buffer::construct((u8*)objects, sizeof(objects)));

}

void MainInterface::onAspectChange(float asp) {
	camera->bind(getParent()->getInfo().getSize(), asp);
}

MainInterface::~MainInterface(){
	g.finish();
	g.destroy(camera);
	g.destroy(sampler);
	g.destroy(rock);
	g.destroy(water);
	g.destroy(osomi);
	g.destroy(mesh);
	g.destroy(mesh0);
	g.destroy(mesh1);
	g.destroy(mesh2);
	g.destroy(mesh3);
	g.destroy(meshBuffer);
	g.destroy(meshBuffer0);
	g.destroy(drawList);
	g.destroy(drawList0);
	g.destroy(renderTarget);
	g.destroy(pipeline);
	g.destroy(pipeline0);
	g.destroy(pipelineState);
	g.destroy(shader);
	g.destroy(shader0);
	g.destroy(cmdList);
}