#pragma once
#include "types/vector.h"
#include "utils/serialization.h"
#include "window/window.h"
#include "graphics/interface/basicgraphicsinterface.h"

namespace oi {

	namespace gc {
		
		class Shader;
		class Pipeline;
		class MeshBuffer;
		class Mesh;
		class DrawList;
		class RenderTarget;
		class MaterialList;
		
		class Texture;
		typedef u32 TextureHandle;
		typedef u32 MaterialHandle;

	}

}

struct Application {
	static void instantiate(oi::wc::WindowHandleExt *param = nullptr);
};

struct NoiseLayer {

	oi::Vec3 offset;
	u32 octaves = 1;
	f32 persistence = 1, roughness = 1, scale = 1, frequency = 1, minValue = 0;
	bool enabled = true, maskLand = false;

	//Returns 0-scale
	f32 sample(oi::Vec3 pos, f32 current);

	ose(NoiseLayer, 0, offset, octaves, persistence, roughness, scale, frequency, minValue, enabled, maskLand);

};

struct Planet {

	oi::Vec3 offset;
	f32 minHeight = -0.6f, scale = 1, coastSize = 0.1f;
	bool randomize = true;

	std::vector<NoiseLayer> noiseLayer = { {} };

	//Returns 0-1 for a point on the planet
	f32 sample(oi::Vec3 pos);

	void seed();

	ose(Planet, 0, minHeight, scale, coastSize, randomize, noiseLayer);

};

class MainInterface : public oi::gc::BasicGraphicsInterface {

public:

	~MainInterface();
	void initScene() override;
	void renderScene() override;
	void onInput(oi::wc::InputDevice *device, oi::wc::Binding b, bool down) override;
	
	void load(oi::String path) override {}
	void save(oi::String path) override {}

	void update(f32 dt) override;
	void initSceneSurface(oi::Vec2u res) override;

	void refreshPlanet(Planet planet);
	void readPlanets(bool fromResource = false);
	void writePlanets();

private:
	
	oi::gc::Shader *shader, *shader0;
	oi::gc::Pipeline *pipeline, *pipeline0;
	oi::gc::MeshBuffer *meshBuffer, *meshBuffer0;
	std::vector<oi::gc::Mesh*> meshes;
	oi::gc::DrawList *drawList, *drawList0;
	oi::gc::RenderTarget *renderTarget;
	oi::gc::MaterialList *materialList;

	oi::gc::Texture *osomi, *water, *rock;
	oi::gc::TextureHandle hwater, hrock;
	oi::gc::MaterialHandle hwaterMat, hrockMat;

	float exposure = .15f, gamma = .85f;
	oi::Vec2 prevMouse;

	struct PerObject {

		oi::Matrix m;
		oi::Matrix mvp;

		oi::Vec3u padding;
		oi::gc::MaterialHandle diffuse;

	};

	static constexpr u32 totalObjects = 2U;

	PerObject objects[totalObjects];

	oi::Vec3 planetRotation;

	std::unordered_map<oi::String, Planet> planets;

};