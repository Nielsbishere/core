#include "graphics/objects/shader/shaderstage.h"
using namespace oi::gc;
using namespace oi;

ShaderStage::ShaderStage(ShaderStageInfo info) : info(info) {}
ShaderStageExt &ShaderStage::getExtension() { return ext; }
const ShaderStageInfo ShaderStage::getInfo() { return info; }