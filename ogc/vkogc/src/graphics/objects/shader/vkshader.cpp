#include "graphics/graphics.h"
#include "graphics/vulkan.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/vkshader.h"
using namespace oi::gc;
using namespace oi;

void Shader::destroyData() {
	g->dealloc(ext);
}

ShaderExt &Shader::getExtension() { return *ext; }

bool Shader::initData() {

	g->alloc(ext);

	ext->stage.resize(info.stage.size());

	for (u32 i = 0; i < (u32) ext->stage.size(); ++i)
		ext->stage[i] = &info.stage[i]->getExtension();

	return true;

}