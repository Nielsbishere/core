#include "graphics/graphics.h"
#include "graphics/objects/view/viewbuffer.h"
using namespace oi::gc;
using namespace oi;

const CameraStruct &Camera::getStruct() { return *info.ptr; }

Camera::~Camera() { 
	info.parent->dealloc(info.ptr); 
	g->destroy(info.parent);
}

Camera::Camera(CameraInfo info) : info(info) {}

bool Camera::init() {

	if ((info.ptr = info.parent->alloc(info.temp)) == nullptr) return false;

	g->use(info.parent);
	handle = info.parent->get(info.ptr); 
	return true;
}

CameraHandle Camera::getHandle() { return handle; }
ViewBuffer *Camera::getParent() { return info.parent; }

void Camera::move(Vec3 dposition) { setPosition(info.ptr->position + dposition); }

void Camera::moveLocal(Vec3 dposition) { 

	Vec3 f = Vec3(info.ptr->forward);

	if (info.ptr->forward.w == 1)
		f = (info.ptr->position - f).normalize();

	Vec3 u = info.ptr->up;
	Vec3 r = f.cross(u);

	move(f * dposition.z + u * dposition.y + r * dposition.x);

}

void Camera::setUp(Vec3 up) { info.ptr->up = up; changed(); }
void Camera::setPosition(Vec3 position) { info.ptr->position = position; changed(); }
void Camera::setDirection(Vec3 direction) { info.ptr->forward = Vec4(direction, 0); changed(); }
void Camera::setCenter(Vec3 center) { info.ptr->forward = Vec4(center, 1); changed(); }

void Camera::changed() {
	info.parent->notify(handle);
}