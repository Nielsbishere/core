#include "window/window.h"
#include "graphics/interface/basicgraphicsinterface.h"
#include "graphics/objects/render/commandlist.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/view/viewbuffer.h"
#include "graphics/objects/model/meshmanager.h"
using namespace oi::gc;
using namespace oi;

void BasicGraphicsInterface::initScene() {

	cmdList = CommandListRef(g, "Default command list", CommandListInfo());

	linearSampler = SamplerRef(g, "Linear sampler", SamplerInfo(SamplerMin::Linear, SamplerMag::Linear, SamplerWrapping::Repeat));
	nearestSampler = SamplerRef(g, "Nearest sampler", SamplerInfo(SamplerMin::Nearest, SamplerMag::Nearest, SamplerWrapping::Clamp_border));

	views = ViewBufferRef(g, "Default view buffer", ViewBufferInfo());
	camera = CameraRef(g, "Default camera", CameraInfo(views, Vec3(0, 0, 3)));
	cameraFrustum = CameraFrustumRef(g, "Default viewport", CameraFrustumInfo(views, Vec2u(1), 1, 40, 0.1f, 100));
	view = ViewRef(g, "Default view", ViewInfo(views, camera, cameraFrustum));

	meshManager = MeshManagerRef(g, "Default mesh manager", MeshManagerInfo(400'000, 500'000, 1024));

}

void BasicGraphicsInterface::onAspectChange(float asp) {
	cameraFrustum->resize(getParent()->getViewport()->getLayer(0).size, asp);
}