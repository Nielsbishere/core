#include "graphics/sampler.h"
using namespace oi::gc;
using namespace oi;

Sampler::Sampler(SamplerInfo info) : info(info) {}
const SamplerInfo Sampler::getInfo() { return info; }
SamplerExt &Sampler::getExtension() { return ext; }