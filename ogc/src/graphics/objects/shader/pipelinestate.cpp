#include "graphics/objects/shader/pipelinestate.h"
using namespace oi::gc;
using namespace oi;

PipelineState::PipelineState(PipelineStateInfo info): info(info){}
const PipelineStateInfo &PipelineState::getInfo() { return info; }