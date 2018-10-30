#include "graphics/viewbuffer.h"
using namespace oi::gc;
using namespace oi;

const ViewStruct &View::getStruct() { return *info.ptr; }

View::~View() { info.parent->dealloc(info.ptr); }
View::View(ViewInfo info) : info(info) {}
bool View::init() { bool b = (info.ptr = info.parent->alloc(info.temp)) != nullptr; handle = info.parent->get(info.ptr); return b; }

ViewHandle View::getHandle() { return handle; }
ViewBuffer *View::getParent() { return info.parent; }

void ViewStruct::makeViewProjection(ViewBuffer *buffer) {
	vp = buffer->getFrustum(frustum)->p * buffer->getCamera(camera)->v;
}
