#include "graphics/objects/view/viewbuffer.h"
using namespace oi::gc;
using namespace oi;

const CameraFrustumStruct &CameraFrustum::getStruct() { return *info.ptr; }

CameraFrustum::~CameraFrustum() { info.parent->dealloc(info.ptr); }
CameraFrustum::CameraFrustum(CameraFrustumInfo info) : info(info) {}
bool CameraFrustum::init() { bool b = (info.ptr = info.parent->alloc(info.temp)) != nullptr; handle = info.parent->get(info.ptr); return b; }

CameraFrustumHandle CameraFrustum::getHandle() { return handle; }
ViewBuffer *CameraFrustum::getParent() { return info.parent; }

void CameraFrustum::resize(Vec2u size) { info.ptr->resolution = size; changed(); }
void CameraFrustum::setAspect(f32 aspect) { info.ptr->aspect = aspect; changed(); }
void CameraFrustum::setNear(f32 near) { info.ptr->near = near; changed(); }
void CameraFrustum::setFar(f32 far) { info.ptr->far = far; changed(); }
void CameraFrustum::setFov(f32 fov) { info.ptr->fov = fov; changed(); }

void CameraFrustum::resize(Vec2u size, f32 aspect) { info.ptr->resolution = size; info.ptr->aspect = aspect; changed(); }

void CameraFrustum::changed() {
	info.parent->notify(128 + handle);
}