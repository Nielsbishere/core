#include "graphics/objects/shader/pipeline.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/pipelinestate.h"
#include "graphics/objects/render/rendertarget.h"
using namespace oi::gc;
using namespace oi;

Pipeline::Pipeline(PipelineInfo info) : info(info) {}
PipelineExt &Pipeline::getExtension() { return ext; }
const PipelineInfo Pipeline::getInfo() { return info; }

bool Pipeline::init() {

	g->use(info.shader);

	if (info.renderTarget != nullptr)
		g->use(info.renderTarget);

	if (info.pipelineState != nullptr)
		g->use(info.pipelineState);

	return initData();
}