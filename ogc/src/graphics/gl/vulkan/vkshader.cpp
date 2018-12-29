#ifdef __VULKAN__

#include "graphics/graphics.h"
#include "graphics/gl/vulkan.h"
#include "graphics/objects/gpubuffer.h"
#include "graphics/objects/shader/shader.h"
#include "graphics/objects/shader/shaderstage.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/texture/versionedtexture.h"
#include "graphics/objects/texture/texturelist.h"
#include "graphics/objects/render/rendertarget.h"
using namespace oi::gc;
using namespace oi;

void Shader::destroyData() {
	g->dealloc<Shader>(ext);
}

ShaderExt &Shader::getExtension() { return *ext; }

bool Shader::initData() {

	g->alloc<Shader>(ext);

	ext->stage.resize(info.stage.size());

	for (u32 i = 0; i < (u32) ext->stage.size(); ++i)
		ext->stage[i] = &info.stage[i]->getExtension();

	return true;

}

#endif