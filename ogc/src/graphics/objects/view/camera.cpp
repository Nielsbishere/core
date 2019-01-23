#include "graphics/graphics.h"
#include "graphics/objects/view/viewbuffer.h"
using namespace oi::gc;
using namespace oi;

CameraStruct::CameraStruct() { memset(this, 0, sizeof(CameraStruct)); }
CameraStruct::CameraStruct(Vec3 position, Vec3 rotation) : position(position), rotation(rotation) {}
const CameraStruct &Camera::getStruct() { return *info.ptr; }

void CameraStruct::makeView() {
	v = Matrix::makeView(position, rotation);
}

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
void Camera::rotate(Vec3 drotation) { 


	setRotation(info.ptr->rotation + drotation); 
}

void Camera::moveLocal(Vec3 dposition) { 
	Matrix &v = info.ptr->v;
	move(v.getForward() * dposition.z + v.getUp() * dposition.y + v.getRight() * -dposition.x);
}

void Camera::setRotation(Vec3 rotation) { info.ptr->rotation = rotation; changed(); }
void Camera::setPosition(Vec3 position) { info.ptr->position = position; changed(); }

void Camera::changed() {
	info.parent->notify(handle);
}