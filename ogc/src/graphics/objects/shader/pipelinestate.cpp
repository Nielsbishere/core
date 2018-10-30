#include "graphics/objects/shader/pipelinestate.h"
using namespace oi::gc;
using namespace oi;

PipelineState::PipelineState(PipelineStateInfo info): info(info){}
PipelineStateExt &PipelineState::getExtension() { return ext; }
const PipelineStateInfo PipelineState::getInfo() { return info; }