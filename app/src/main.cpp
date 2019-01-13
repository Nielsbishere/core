#include "main.h"
#include "utils/random.h"
#include "file/filemanager.h"
#include "math/simplexnoise.h"
#include "window/windowmanager.h"
#include "input/inputhandler.h"
#include "input/mouse.h"
#include "input/keyboard.h"
#include "graphics/format/oirm.h"
#include "graphics/format/fbx.h"
#include "graphics/objects/model/mesh.h"

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

//Set up a main window
void Application::instantiate(AppExt *param){
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

void MainInterface::refreshPlanet(Planet planet, MeshAllocationInfo &mai) {

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

		*(Vec3*)(avertex + 8 * i) = spos * (1 + planet.sample(spos));		//Displacement on sphere
		*(Vec2*)(avertex + 8 * i + 3) = uv;									//Temporary planar projection
		*(Vec3*)(avertex + 8 * i + 5) = spos;								//Normal = position for a sphere

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

	mai = MeshAllocationInfo("Planet", mesh3D, { Buffer((u8*)avertex, vertices * 32) }, Buffer((u8*)aindex, indices * 4));

	t.stop();
	t.print();

}

void MainInterface::writePlanets() {

	JSON json;
	json.serialize(planets, true);
	FileManager::get()->write("out/models/planets.json", json.toString());

	RMFile file = oiRM::convert(meshes[4]->getInfo());
	oiRM::write(file, String("out/models/") + meshes[4]->getName() + ".oiRM", false);
}

void MainInterface::readPlanets(bool fromResource, MeshAllocationInfo &mai) {

	String str;
	FileManager::get()->read(String(fromResource ? "res/models/planets.json" : "out/models/planets.json"), str);
	JSON json = str;
	json.serialize(planets, false);

	for(auto &elem : planets)
		refreshPlanet(elem.second, mai);

}

void MainInterface::refreshPlanetMesh(bool fromResource) {

	MeshAllocationInfo mai;
	readPlanets(fromResource, mai);

	if (meshes[4] != nullptr)
		meshManager->unload(meshes[4]);

	meshes[4] = meshManager->load(mai);

}

void MainInterface::initScene() {

	BasicGraphicsInterface::initScene();
	Log::println("Started main interface!");

	FileManager::get()->foreachFileRecurse("res", [](FileInfo info) -> bool { Log::println(info.name); return false; });

	//Setup our input manager

	getInputManager().load("res/settings/input.json");

	//Setup shaders

	deferred = ShaderRef(g, "Deferred pass", ShaderInfo("res/shaders/simple.graphics.oiSH"));
	postProcessing = ShaderRef(g, "Post processing pass", ShaderInfo("res/shaders/post_process.graphics.oiSH"));
	lighting = ShaderRef(g, "Lighting pass", ShaderInfo("res/shaders/cmi_lighting.compute.oiSH"));

	//Setup render targets

	gbuffer = RenderTargetRef(g, "G-Buffer target",
		RenderTargetInfo(Vec2u(), TextureFormat::Depth, { 
			TextureFormat::RG16f,	//uv 4 Bpp
			TextureFormat::RG16,	//normal 4 Bpp
			TextureFormat::R16u		//material 2 Bpp
		})
	);

	lightingTarget = RenderTargetRef(g, "Lighting target", RenderTargetInfo(Vec2u(), { TextureFormat::RGBA16f }));

	//First: Allocate anvil into new MeshBuffer (with default size)
	//Second: Allocate sword into existing MeshBuffer
	//Third: Allocate sphere into existing MeshBuffer
	//Last: Allocate quad into new MeshBuffer (same size as mesh)

	std::vector<MeshAllocationInfo> info = {
		{ "res/models/anvil.oiRM", MeshAllocationHint::ALLOCATE_DEFAULT },
		{ "res/models/sword.oiRM" },
		{ "res/models/sphere.oiRM" },
		{ "res/models/quad.oiRM", MeshAllocationHint::SIZE_TO_FIT }
	};

	//Load our models

	meshes = meshManager->loadAll(info);
	meshes.push_back(nullptr);				//Reserve planet

	//Get our mesh buffers

	mesh3D = meshes[0]->getBuffer();
	mesh2D = meshes[3]->getBuffer();

	//Load planet

	refreshPlanetMesh(true);

	//Setup pipelines

	pipelineState = PipelineStateRef(g, "Default pipeline state", PipelineStateInfo(DepthMode::All, BlendMode::Off));

	deferredPipeline = PipelineRef(g, "Deferred pipeline", GraphicsPipelineInfo(deferred, pipelineState, gbuffer, mesh3D));
	lightingPipeline = PipelineRef(g, "Lighting pipeline", ComputePipelineInfo(lighting));
	postProcessingPipeline = PipelineRef(g, "Post process pipeline", GraphicsPipelineInfo(postProcessing, pipelineState, g.getBackBuffer(), mesh2D));

	//Create draw and compute lists

	lightingDispatch = ComputeListRef(g, "CMI lighting compute list", ComputeListInfo(lightingPipeline, 1));
	drawList = DrawListRef(g, "Draw list (main geometry)", DrawListInfo(mesh3D, 2, false));
	quad = DrawListRef(g, "Draw list (post processing quad)", DrawListInfo(mesh2D, 1, false));

	//Setup geometry draw calls

	drawList->draw(meshes[2], 1);		//Reserve objects[0] for the sphere/water
	drawList->draw(meshes[4], 1);		//Reserve objects[1] for the planet
	drawList->flush();

	//Setup our post processing pass to draw a quad

	quad->draw(meshes[3], 1);
	quad->flush();

	//Create our textures

	textureList = TextureListRef(g, "Textures", TextureListInfo(2));

	trock = TextureRef(g, "Rock", TextureInfo(textureList, "res/textures/rock_dif.png"));
	twater = TextureRef(g, "Water", TextureInfo(textureList, "res/textures/water_dif.png"));

	//Create the materials

	materialList = MaterialListRef(g, "Materials", MaterialListInfo(textureList, 2));

	rock = MaterialRef(g, "Rock material", MaterialInfo(materialList));
	rock->setDiffuse(trock.get());

	water = MaterialRef(g, "Water material", MaterialInfo(materialList));
	water->setDiffuse(twater.get());

	//Set our materials in our objects array

	objects[0].diffuse = water->getHandle();	//Water
	objects[1].diffuse = rock->getHandle();		//Planet

	//Clustered material indirect lighting

	lightingPipeline->setRegister("tex", textureList);
	lightingPipeline->setRegister("samp", linearSampler);
	lightingPipeline->setRegister("nearest", nearestSampler);

	//Setup shader data

	deferredPipeline->instantiateBuffer("Objects", totalObjects);

	postProcessingPipeline->setRegister("samp", nearestSampler);

	lightingPipeline->setBuffer("Views", 0, views->getBuffer());
	lightingPipeline->setBuffer("Materials", materialList->getSize(), materialList->getBuffer());
	lightingPipeline->instantiateBuffer("PointLights", 1);
	lightingPipeline->instantiateBuffer("SpotLights", 1);
	lightingPipeline->instantiateBuffer("DirectionalLights", 1);
	
	lightingPipeline->setValue("DirectionalLights/light/dir", Vec3(-1, 0, -1));
	lightingPipeline->setValue("DirectionalLights/light/intensity", 16.f);
	lightingPipeline->setValue("DirectionalLights/light/col", Vec3(1.f));

	lightingPipeline->setRegister("outputTexture", lightingTarget->getTarget(0));
	lightingPipeline->setRegister("uvs", gbuffer->getTarget(0));
	lightingPipeline->setRegister("normals", gbuffer->getTarget(1));
	lightingPipeline->setRegister("materials", gbuffer->getTarget(2));
	lightingPipeline->setRegister("depth", gbuffer->getDepth());

	lightingPipeline->setValue("Global/ambient", Vec3(1));
	lightingPipeline->setValue("Global/power", 1.f);
	lightingPipeline->setValue("Global/view", view->getHandle());

	postProcessingPipeline->setRegister("tex", lightingTarget->getTarget(0));
}

void MainInterface::renderScene(){

	//Start frame

	cmdList->begin();

	//Render to g-buffer

	cmdList->begin(gbuffer);
	cmdList->bind(deferredPipeline);
	cmdList->draw(drawList);
	cmdList->end(gbuffer);

	//Clustered Material Indirect Rendering

	cmdList->begin(lightingTarget);
	cmdList->bind(lightingPipeline);
	cmdList->dispatch(lightingDispatch);
	cmdList->end(lightingTarget);

	//Copy to back buffer & do post processing

	cmdList->begin(g.getBackBuffer());
	cmdList->bind(postProcessingPipeline);
	cmdList->draw(quad);
	cmdList->end(g.getBackBuffer());

	//Push frame

	cmdList->end();

}

void MainInterface::initSceneSurface(Vec2u res){

	lightingTarget->resize(res);
	gbuffer->resize(res);

	lightingPipeline->setValue("Global/resolution", res);
	lightingPipeline->update();
	deferredPipeline->update();
	postProcessingPipeline->update();

	lightingDispatch->clear();
	lightingDispatch->dispatchThreads(res);
	lightingDispatch->flush();

}

void MainInterface::onInput(InputDevice*, Binding b, bool down) {

	if (b.getBindingType() == BindingType::KEYBOARD && down) {

		if (b.toKey() == Key::Volume_up || b.toKey() == Key::Up)
			writePlanets();
		else if (b.toKey() == Key::Volume_down || b.toKey() == Key::Down)
			refreshPlanetMesh(true);
		else if (b.toKey() == Key::Enter)
			g.printObjects();

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
	}

	prevMouse = nextMouse;
	planetRotation += Vec3(30, 50) * dt;

	//Force view buffer to update matrices of cameras, viewports and views
	views->update();

	//Update planet rotation

	objects[0].m = Matrix::makeModel(Vec3(), Vec3(planetRotation, 0.f), Vec3(1.5f));
	objects[0].mvp = view->getStruct().vp * objects[0].m;

	objects[1].m = Matrix::makeModel(Vec3(), Vec3(planetRotation, 0.f), Vec3(3.f));
	objects[1].mvp = view->getStruct().vp * objects[1].m;

	deferredPipeline->setData("Objects", Buffer::construct((u8*)objects, sizeof(objects)));

	//Update time

	lightingPipeline->setValue("Global/time", (f32)getRuntime());

	//Setup post processing settings

	postProcessingPipeline->setValue("PostProcessingSettings/exposure", exposure);
	postProcessingPipeline->setValue("PostProcessingSettings/gamma", gamma);

}

MainInterface::~MainInterface(){
	g.finish();
}