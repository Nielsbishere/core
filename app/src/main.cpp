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
#include "graphics/objects/model/meshbuffer.h"
#include "graphics/objects/model/mesh.h"
#include "graphics/objects/model/materiallist.h"
#include "graphics/objects/model/material.h"
#include "graphics/objects/model/meshmanager.h"
#include "graphics/objects/render/drawlist.h"
#include "graphics/objects/shader/computelist.h"
#include "graphics/objects/render/rendertarget.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/view/viewbuffer.h"
#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/texture/versionedtexture.h"

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

	mai = MeshAllocationInfo("Planet", meshBuffer, { Buffer((u8*)avertex, vertices * 32) }, Buffer((u8*)aindex, indices * 4));

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

	//Setup our input manager

	getInputManager().load("res/settings/input.json");

	//Setup our shader (forward phong shading)

	shader = g.create("Simple", ShaderInfo("res/shaders/simple.oiSH"));
	g.use(shader);

	shader->get<ShaderBuffer>("Objects")->instantiate(totalObjects);

	//Setup our post process shader (tone mapping & gamma correction)

	shader0 = g.create("Post process", ShaderInfo("res/shaders/post_process.oiSH"));
	g.use(shader0);

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

	meshBuffer = meshes[0]->getBuffer();
	meshBuffer0 = meshes[3]->getBuffer();

	//Read in planet model

	refreshPlanetMesh(true);

	//Set up our draw list

	drawList = g.create("Draw list (main geometry)", DrawListInfo(meshBuffer, 256, false));
	g.use(drawList);

	drawList0 = g.create("Draw list (post processing pass)", DrawListInfo(meshBuffer0, 1, false));
	g.use(drawList0);

	//Setup our post processing pass to draw a quad

	drawList0->draw(meshes[3], 1);
	drawList0->flush();

	//Setup geometry draw calls

	drawList->draw(meshes[2], 1);		//Reserve objects[0] for the sphere/water
	drawList->draw(meshes[4], 1);		//Reserve objects[1] for the planet
	drawList->flush();

	//Allocate our textures into a TextureList

	textureList = g.create("Textures", TextureListInfo(2));

	//Allocate textures

	trock = g.create("rock", TextureInfo(textureList, "res/textures/rock_dif.png"));
	g.use(trock);

	twater = g.create("water", TextureInfo(textureList, "res/textures/water_dif.png"));
	g.use(twater);

	//Create the material list

	materialList = g.create("Materials", MaterialListInfo(textureList, 2));
	g.use(materialList);

	//Setup our materials

	rock = g.create("Rock material", MaterialInfo(materialList));
	rock->setDiffuse(trock);

	water = g.create("Water material", MaterialInfo(materialList));
	water->setDiffuse(twater);

	//Set our materials in our objects array

	objects[0].diffuse = water->getHandle();	//Water
	objects[1].diffuse = rock->getHandle();		//Planet

	//Set our shader samplers

	shader0->set("samp", linearSampler);

	//Setup ourpipeline state

	pipelineState = g.create("Default pipeline state", PipelineStateInfo(DepthMode::All, BlendMode::Off));
	g.use(pipelineState);

	//Clustered material indirect lighting

	cmiLighting = g.create("CMI lighting", ShaderInfo("res/shaders/cmi_lighting.oiSH"));
	g.use(cmiLighting);

	cmiLighting->set("tex", textureList);
	cmiLighting->set("samp", linearSampler);
	cmiLighting->set("nearest", nearestSampler);

	cmiLightingPipeline = g.create("CMI lighting pipeline", PipelineInfo(cmiLighting));
	g.use(cmiLightingPipeline);

	cmiLightingDispatch = g.create("CMI lighting compute list", ComputeListInfo(cmiLightingPipeline, 1));
	g.use(cmiLightingDispatch);

	//Set our view data to our view buffer
	//Set our material data to material buffer
	//Set our object data to our object buffer

	ShaderBuffer *shaderViews = cmiLighting->get<ShaderBuffer>("Views");

	if(shaderViews != nullptr)
		shaderViews->setBuffer(0, views->getBuffer());

	ShaderBuffer *materials = cmiLighting->get<ShaderBuffer>("Materials");

	if(materials != nullptr)
		materials->setBuffer(materialList->getSize(), materialList->getBuffer());

	//Setup lighting; 1 point, directional and spot light

	ShaderBuffer *pointLights = cmiLighting->get<ShaderBuffer>("PointLights");
	
	if(pointLights != nullptr)
		pointLights->instantiate(1);

	ShaderBuffer *spotLights = cmiLighting->get<ShaderBuffer>("SpotLights");
	
	if(spotLights != nullptr)
		spotLights->instantiate(1);

	ShaderBuffer *directionalLights = cmiLighting->get<ShaderBuffer>("DirectionalLights");

	//Pass directional data (no point/spot lights)

	if (directionalLights != nullptr) {
		directionalLights->instantiate(1);
		directionalLights->set("light/dir", Vec3(-1, 0, -1));
		directionalLights->set("light/intensity", 16.f);
		directionalLights->set("light/col", Vec3(1.f));
	}
}

void MainInterface::renderScene(){

	//Start frame

	cmdList->begin();

	//Render to g-buffer

	cmdList->begin(renderTarget);
	cmdList->bind(pipeline);
	cmdList->draw(drawList);
	cmdList->end(renderTarget);

	//Clustered Material Indirect Rendering

	cmdList->begin(cmiLightingTarget);
	cmdList->bind(cmiLightingPipeline);
	cmdList->dispatch(cmiLightingDispatch);
	cmdList->end(cmiLightingTarget);

	//Copy to back buffer & do post processing

	cmdList->begin(g.getBackBuffer());
	cmdList->bind(pipeline0);
	cmdList->draw(drawList0);
	cmdList->end(g.getBackBuffer());

	//Push frame

	cmdList->end();

}

void MainInterface::initSceneSurface(Vec2u res){

	//Destroy old data
	g.destroy(renderTarget);
	g.destroy(cmiLightingTarget);
	g.destroy(pipeline);
	g.destroy(pipeline0);

	if (res == Vec2u())
		return;

	//Recreate render targets and pipelines

	//Create G-Buffer; 10 Bpp
	//RG16f = uv; 4 Bpp
	//RG16 = normal; 4 Bpp
	//R16u = material id 2 Bpp

	g.use(renderTarget = g.create("G-Buffer target", 
		RenderTargetInfo(res, TextureFormat::Depth, { TextureFormat::RG16f, TextureFormat::RG16, TextureFormat::R16u })
	));

	//drawList -> Rendering pipeline -> renderTarget

	pipeline = g.create("Rendering pipeline", PipelineInfo(shader, pipelineState, renderTarget, meshBuffer));
	g.use(pipeline);

	//renderTarget -> Post processing pipeline -> back buffer

	pipeline0 = g.create("Post process pipeline", PipelineInfo(shader0, pipelineState, g.getBackBuffer(), meshBuffer0));
	g.use(pipeline0);

	//Resize compute parameters

	cmiLightingDispatch->clear();
	cmiLightingDispatch->dispatchThreads(res);
	cmiLightingDispatch->flush();

	cmiLightingTarget = g.create("CMI lighting target", RenderTargetInfo(res, { TextureFormat::RGBA16f }));
	g.use(cmiLightingTarget);

	cmiLighting->set("outputTexture", cmiLightingTarget->getTarget(0));
	cmiLighting->set("uvs", renderTarget->getTarget(0));
	cmiLighting->set("normals", renderTarget->getTarget(1));
	cmiLighting->set("materials", renderTarget->getTarget(2));
	cmiLighting->set("depth", renderTarget->getDepth());

	ShaderBuffer *global = cmiLighting->get<ShaderBuffer>("Global");
	global->set("resolution", res);

	//Update post processing shader

	shader0->set("tex", cmiLightingTarget->getTarget(0))
}
	
void MainInterface::onInput(InputDevice*, Binding b, bool down) {

	if (b.getBindingType() == BindingType::KEYBOARD && down) {

		if (b.toKey() == Key::Volume_up || b.toKey() == Key::Up)
			writePlanets();
		else if (b.toKey() == Key::Volume_down || b.toKey() == Key::Down)
			refreshPlanetMesh(true);

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

	shader->get<ShaderBuffer>("Objects")->set(Buffer::construct((u8*)objects, sizeof(objects)));

	//Update global shader buffer
	ShaderBuffer *global = cmiLighting->get<ShaderBuffer>("Global");
	global->set("ambient", Vec3(1));
	global->set("time", (f32)getRuntime());
	global->set("power", 1.f);
	global->set("view", view->getHandle());

	//Setup post processing settings
	ShaderBuffer *postProcessing = shader0->get<ShaderBuffer>("PostProcessingSettings");

	postProcessing->set("exposure", exposure);
	postProcessing->set("gamma", gamma);

}

MainInterface::~MainInterface(){
	g.finish();
	g.destroy(renderTarget);
	g.destroy(cmiLightingTarget);
	g.destroy(pipeline);
	g.destroy(pipeline0);
	g.destroy(pipelineState);
	g.destroy(rock);
	g.destroy(water);
	g.destroy(trock);
	g.destroy(twater);
	g.destroy(textureList);
	g.destroy(materialList);
	g.destroy(cmiLightingDispatch);
	g.destroy(drawList);
	g.destroy(drawList0);
	g.destroy(renderTarget);
	g.destroy(pipeline);
	g.destroy(pipeline0);
	g.destroy(cmiLightingPipeline);
	g.destroy(shader);
	g.destroy(shader0);
	g.destroy(cmiLighting);
}