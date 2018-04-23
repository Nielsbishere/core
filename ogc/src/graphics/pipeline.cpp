#include "graphics/pipeline.h"
using namespace oi::gc;
using namespace oi;

Pipeline::Pipeline(PipelineInfo info) : info(info) {}
PipelineExt &Pipeline::getExtension() { return ext; }
const PipelineInfo &Pipeline::getInfo() { return info; }