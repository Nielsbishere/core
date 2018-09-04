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
#include <utils/json.h>

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

float NoiseLayer::sample(Vec3 pos) {
	float value = SimplexNoise::noise(pos, offset, octaves, persistence, roughness) * scale * (mask == nullptr ? 1 : mask->sample(pos));
	return value > minValue ? (value - minValue) / (scale - minValue) : 0;
}

float NoiseLayer::getScale() {
	float val = scale * (mask == nullptr ? 1 : mask->getScale()) - minValue;
	return val > 0 ? val : 0;
}

void NoiseLayer::serialize(JSON &json, String path, bool save) {
	json.serialize(path + "/offset", offset, save);
	json.serialize(path + "/octaves", octaves, save);
	json.serialize(path + "/persistence", persistence, save);
	json.serialize(path + "/roughness", roughness, save);
	json.serialize(path + "/scale", scale, save);
	json.serialize(path + "/minValue", minValue, save);
	json.serialize(path + "/enabled", enabled, save);
}

//Returns 0-1 for a point on the planet
float Planet::sample(Vec3 pos) {

	float val = 0, scale = 0;

	for (u32 i = 0; i < (u32)noiseLayer.size(); ++i)
		if (noiseLayer[i].enabled) {
			val += noiseLayer[i].sample(pos);
			scale += noiseLayer[i].getScale();
		}

	return val / scale * (maxHeight - minHeight) + minHeight;

}

void Planet::serialize(JSON &json, String path, bool save) {
	json.serialize(path + "/minHeight", minHeight, save);
	json.serialize(path + "/maxHeight", maxHeight, save);
	json.serialize(path + "/layer", noiseLayer, save);
}

void MainInterface::refreshPlanet(Planet planet) {

	Timer t;

	u32 resolution = 32;

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

	FileManager::get()->write(String("out/models/") + planet.name + ".oiRM", buf);

	buf.deconstruct();

	if (mesh3 != nullptr)
		g.destroy(mesh3);

	meshBuffer->open();

	RMFile file;
	oiRM::read(String("out/models/") + planet.name + ".oiRM", file);
	auto info = oiRM::convert(&g, file);

	info.second.buffer = meshBuffer;
	mesh3 = g.create(planet.name, info.second);
	g.use(mesh3);

	meshBuffer->close();

	t.stop();
	t.print();

}

void MainInterface::writePlanet(Planet planet) {
	JSON json;
	json.serialize("", planet, true);
	FileManager::get()->write(String("out/") + planet.name  +".json", json.toString());
	RMFile file = oiRM::convert(mesh3->getInfo());
	oiRM::write(file, String("out/models/") + planet.name + ".oiRM", false);
}

void MainInterface::readPlanet(Planet &planet, String name) {
	String str;
	FileManager::get()->read(String("out/") + name + ".json", str);
	JSON json = str;
	json.serialize(name, planet, false);
	refreshPlanet(planet);
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
	auto info = oiRM::convert(&g, file);

	info.first.maxIndices = 300000;
	info.first.maxVertices = 200000;
	info.first.topologyMode = TopologyMode::Triangle;
	info.first.fillMode = FillMode::Line;
	meshBuffer = g.create("Mesh buffer", info.first);
	g.use(meshBuffer);
	meshBuffer->open();

	info.second.buffer = meshBuffer;
	mesh = g.create("Anvil", info.second);
	g.use(mesh);

	oiRM::read("res/models/sword.oiRM", file);
	info = oiRM::convert(&g, file);

	info.second.buffer = meshBuffer;
	mesh0 = g.create("Sword", info.second);
	g.use(mesh0);

	oiRM::read("res/models/sphere.oiRM", file);
	info = oiRM::convert(&g, file);

	info.second.buffer = meshBuffer;
	mesh2 = g.create("Sphere", info.second);
	g.use(mesh2);

	meshBuffer->close();

	refreshPlanet(earth);

	//Setup our quad
	oiRM::read("res/models/post_processing_quad.oiRM", file);
	info = oiRM::convert(&g, file);
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
	camera = g.create("Default camera", CameraInfo(45.f, Vec3(3, 3, 3), Vec4(0, 0, 0, 1)));
	g.use(camera);

	//Setup lighting
	ShaderBuffer *lights = shader->get<ShaderBuffer>("Lights");

	lights->open();

	lights->set("directional/dir", Vec3(-1, -1, -1));
	lights->set("directional/intensity", 0.5f);
	lights->set("directional/col", Vec3(1.f));

	lights->set("point/pos", Vec3());
	lights->set("point/intensity", 2.f);
	lights->set("point/radius", 4.f);
	lights->set("point/col", Vec3(1.f));

	lights->set("spot/pos", Vec3(0, 1, 0));
	lights->set("spot/dir", Vec3(1, 1, 1));
	lights->set("spot/intensity", 8.f);
	lights->set("spot/angle", 40.f);
	lights->set("spot/radius", 4.f);
	lights->set("spot/col", Vec3(1.f));

	lights->close();

	//Setup drawcalls  (reserve objects for meshes)

	drawList->draw(mesh0, totalObjects / 4);		//Reserve 0 until (not including) totalObjects/4
	drawList->draw(mesh, totalObjects / 4);			//Reserve 1/4 until 2/4
	drawList->draw(mesh2, totalObjects / 4);		//Reserve 2/4 until 3/4
	drawList->draw(mesh3, totalObjects / 4);		//Reserve 3/4 until 4/4
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
			writePlanet(earth);
		else if (b.toKey() == Key::Volume_down || b.toKey() == Key::Down)
			readPlanet(earth, "earth");

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

	camera->bind(getParent()->getInfo().getResolution());

	for (u32 i = 0; i < totalObjects; ++i) {
		objects[i].m = Matrix::makeScale(Vec4(0, 0, 0, 1));// Matrix::makeModel(Vec3((f32)i / totalObjects * 6 - 3, 0, 0), Vec3(planetRotation, 0.f), Vec3(2.f) / (i + 1));
		objects[i].mvp = { camera->getBoundProjection() * camera->getBoundView() * objects[i].m };
	}

	objects[totalObjects - 1].m = Matrix::makeModel(Vec3(0, 0, 0), Vec3(planetRotation, 0.f), Vec3(1.5f));
	objects[totalObjects - 1].mvp = { camera->getBoundProjection() * camera->getBoundView() * objects[totalObjects - 1].m };

	shader->get<ShaderBuffer>("Objects")->getBuffer()->set(Buffer::construct((u8*)objects, sizeof(objects)));

}

MainInterface::~MainInterface(){
	g.finish();
	g.destroy(camera);
	g.destroy(sampler);
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