#include "graphics/sampler.h"
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi;

Sampler::~Sampler() {
	vkDestroySampler(g->getExtension().device, ext, allocator);
}

bool Sampler::init() {

	VkSamplerCreateInfo samplerInfo;
	memset(&samplerInfo, 0, sizeof(samplerInfo));

	GraphicsExt gext = g->getExtension();

	if ((!g->getExtension().pfeatures.samplerAnisotropy && info.aniso != 1.f) || info.aniso < 1.f) {
		Log::warn("Sampler aniso value was invalid; because it is not be supported. Resetting it.");
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

	vkCheck<0x0, Sampler>(vkCreateSampler(g->getExtension().device, &samplerInfo, allocator, &ext), "Couldn't create sampler object");

	return true;
}