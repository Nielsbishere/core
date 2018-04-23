#include "graphics/shaderstage.h"
using namespace oi::gc;
using namespace oi;

ShaderStage::ShaderStage(ShaderStageInfo info) : info(info) {}
ShaderStageExt &ShaderStage::getExtension() { return ext; }