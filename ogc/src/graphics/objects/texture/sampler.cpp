#include "graphics/objects/texture/sampler.h"
using namespace oi::gc;
using namespace oi;

Sampler::Sampler(SamplerInfo info) : info(info) {}
const SamplerInfo Sampler::getInfo() { return info; }