#include "graphics/shaderstage.h"
using namespace oi::gc;
using namespace oi;

ShaderStage::ShaderStage(String name, ShaderStageType type): path(String("res/shaders/") + name), type(type) { }
ShaderStage::ShaderStage(String name) : ShaderStage(name, ShaderStageType(*(name.split(".").end() - 1))) {}