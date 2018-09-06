#include <types/matrix.h>
#include "graphics/camera.h"
#include "graphics/pipeline.h"
#include "graphics/rendertarget.h"

#undef near
#undef far

using namespace oi::gc;
using namespace oi;

const CameraInfo Camera::getInfo() { return info; }

void Camera::bind(Vec2u res, f32 aspect) {

	Vec3 center;

	if (info.directionOrCenter.w == 1)
		center = Vec3(info.directionOrCenter);
	else
		center = info.position + Vec3(info.directionOrCenter);

	Vec3 forward = center - info.position;

	if (forward != Vec3())
		forward = forward.normalize();
	else
		Log::throwError<Camera, 0x2>("Camera requires valid forward");

	bound = CameraStruct(Matrix::makePerspective(info.fov, aspect, info.near, info.far), Matrix::makeView(info.position, center, info.up), info.position, info.fov, info.up, aspect, forward, info.near, info.far, res);
}

CameraStruct Camera::getBound() const { return bound; }
Matrix Camera::getBoundProjection() const { return bound.p; }
Matrix Camera::getBoundView() const { return bound.v; }
Camera::~Camera() {}
Camera::Camera(CameraInfo info) : info(info) {}
bool Camera::init() { return true; }

void Camera::move(Vec3 dposition) { info.position += dposition; }

void Camera::setPosition(Vec3 position) { info.position = position; }
void Camera::setFov(f32 fov) { info.fov = fov; }
void Camera::setNear(f32 near) { info.near = near; }
void Camera::setFar(f32 far) { info.far = far; }
void Camera::setUp(Vec3 up) { info.up = up; }
void Camera::setDirection(Vec3 direction) { info.directionOrCenter = Vec4(direction, 0.f); }
void Camera::setCenter(Vec3 center) { info.directionOrCenter = Vec4(center, 1.f); }
