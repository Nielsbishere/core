#include "window/window.h"
#include "graphics/interface/basicgraphicsinterface.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/view/viewbuffer.h"
#include "graphics/objects/model/meshmanager.h"
using namespace oi::gc;
using namespace oi;

BasicGraphicsInterface::~BasicGraphicsInterface() {
	g.destroy(meshManager);
	g.destroy(view);
	g.destroy(cameraFrustum);
	g.destroy(camera);
	g.destroy(views);
	g.destroy(sampler);
	g.destroy(pipelineState);
	g.destroy(cmdList);
}

void BasicGraphicsInterface::initScene() {

	//Command list
	cmdList = g.create("Default command list", CommandListInfo());
	g.use(cmdList);

	//Setup our pipeline state (with default settings)
	pipelineState = g.create("Default pipeline state", PipelineStateInfo());
	g.use(pipelineState);

	//Allocate sampler
	sampler = g.create("Default sampler", SamplerInfo(SamplerMin::Linear, SamplerMag::Linear, SamplerWrapping::Repeat));
	g.use(sampler);

	//Setup our view buffer
	views = g.create("Default view buffer", ViewBufferInfo());
	g.use(views);

	//Setup our camera
	camera = g.create("Default camera", CameraInfo(views, Vec3(3), Vec4(0, 0, 0, 1)));
	g.use(camera);

	//Setup our viewport
	cameraFrustum = g.create("Default viewport", CameraFrustumInfo(views, Vec2u(1), 1, 40, 0.1f, 100));
	g.use(cameraFrustum);

	//Setup our view
	view = g.create("Default view", ViewInfo(views, camera, cameraFrustum));
	g.use(view);

	//Setup our mesh manager
	meshManager = g.create("Default mesh manager", MeshManagerInfo(400'000, 500'000));
	g.use(meshManager);

}

void BasicGraphicsInterface::onAspectChange(float asp) {
	cameraFrustum->resize(getParent()->getInfo().getSize(), asp);
}