#include "graphics/objects/shader/shaderstage.h"
using namespace oi::gc;
using namespace oi;

ShaderStage::ShaderStage(ShaderStageInfo info) : info(info) {}
const ShaderStageInfo &ShaderStage::getInfo() const { return info; }