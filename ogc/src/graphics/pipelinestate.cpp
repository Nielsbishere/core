#include "graphics/pipelinestate.h"
using namespace oi::gc;
using namespace oi;

PipelineState::PipelineState(PipelineStateInfo info): info(info){}
PipelineStateExt &PipelineState::getExtension() { return ext; }