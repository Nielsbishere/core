#include "graphics/graphics.h"
#include "graphics/generic.h"
#include "graphics/vulkan.h"
#include "graphics/objects/texture/sampler.h"
#include "graphics/objects/texture/vksampler.h"
using namespace oi::gc;
using namespace oi;

Sampler::~Sampler() {
	vkDestroySampler(g->getExtension().device, ext->obj, vkAllocator);
	g->dealloc<Sampler>(ext);
}

SamplerExt &Sampler::getExtension() { return *ext; }

bool Sampler::init() {

	g->alloc<Sampler>(ext);

	VkSamplerCreateInfo samplerInfo;
	memset(&samplerInfo, 0, sizeof(samplerInfo));

	GraphicsExt gext = g->getExtension();

	if ((!g->getExtension().pfeatures.samplerAnisotropy && info.aniso != 1.f) || info.aniso < 1.f) {
		Log::warn("Sampler aniso value was invalid; because it is not be supported. Resetting it");
		info.aniso = 1U;
	}

	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.minFilter = SamplerMinExt(info.minFilter.getName()).getValue().filter;
	samplerInfo.mipmapMode = SamplerMinExt(info.minFilter.getName()).getValue().mip;
	samplerInfo.magFilter = SamplerMagExt(info.magFilter.getName()).getValue();
	samplerInfo.maxAnisotropy = (float) info.aniso;
	samplerInfo.addressModeU = SamplerWrappingExt(info.r.getName()).getValue();
	samplerInfo.addressModeV = SamplerWrappingExt(info.s.getName()).getValue();
	samplerInfo.addressModeW = SamplerWrappingExt(info.t.getName()).getValue();
	samplerInfo.anisotropyEnable = info.aniso > 1.f;
	samplerInfo.maxLod = 32.f;

	vkCheck<0x0, VkSampler>(vkCreateSampler(g->getExtension().device, &samplerInfo, vkAllocator, &ext->obj), "Couldn't create sampler object");
	vkName(gext, ext->obj, VK_OBJECT_TYPE_SAMPLER, getName());

	return true;
}