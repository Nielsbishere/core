#include "graphics/objects/view/viewbuffer.h"
using namespace oi::gc;
using namespace oi;

const CameraStruct &Camera::getStruct() { return *info.ptr; }

Camera::~Camera() { info.parent->dealloc(info.ptr); }
Camera::Camera(CameraInfo info) : info(info) {}
bool Camera::init() { bool b = (info.ptr = info.parent->alloc(info.temp)) != nullptr; handle = info.parent->get(info.ptr); return b; }

CameraHandle Camera::getHandle() { return handle; }
ViewBuffer *Camera::getParent() { return info.parent; }

void Camera::move(Vec3 dposition) { setPosition(info.ptr->position + dposition); }
void Camera::setUp(Vec3 up) { info.ptr->up = up; changed(); }
void Camera::setPosition(Vec3 position) { info.ptr->position = position; changed(); }
void Camera::setDirection(Vec3 direction) { info.ptr->forward = Vec4(direction, 0); changed(); }
void Camera::setCenter(Vec3 center) { info.ptr->forward = Vec4(center, 1); changed(); }

void Camera::changed() {
	info.parent->notify(handle);
}