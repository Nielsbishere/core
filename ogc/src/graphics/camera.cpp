#include <types/matrix.h>
#include "graphics/camera.h"
#include "graphics/pipeline.h"
#include "graphics/rendertarget.h"

#undef near
#undef far

using namespace oi::gc;
using namespace oi;

const CameraInfo Camera::getInfo() { return info; }

void Camera::bind(Vec2u res) {

	Vec3f center;

	if (info.directionOrCenter.w == 1)
		center = Vec3f(info.directionOrCenter);
	else
		center = info.position + Vec3(info.directionOrCenter);

	Vec3f forward = center - info.position;

	if (forward != Vec3f())
		forward = forward.normalize();
	else
		Log::throwError<Camera, 0x2>("Camera requires valid forward");

	f32 aspect = Vec2f(res).getAspect();

	bound = CameraStruct(Matrixf::makePerspective(info.fov, aspect, info.near, info.far), Matrixf::makeView(info.position, center, info.up), info.position, info.fov, info.up, aspect, forward, info.near, info.far, res);
}

CameraStruct Camera::getBound() const { return bound; }
Matrixf Camera::getBoundProjection() const { return bound.p; }
Matrixf Camera::getBoundView() const { return bound.v; }
Camera::~Camera() {}
Camera::Camera(CameraInfo info) : info(info) {}
bool Camera::init() { return true; }

void Camera::move(Vec3f dposition) { info.position += dposition; }

void Camera::setPosition(Vec3f position) { info.position = position; }
void Camera::setFov(f32 fov) { info.fov = fov; }
void Camera::setNear(f32 near) { info.near = near; }
void Camera::setFar(f32 far) { info.far = far; }
void Camera::setUp(Vec3f up) { info.up = up; }
void Camera::setDirection(Vec3f direction) { info.directionOrCenter = Vec4f(direction, 0.f); }
void Camera::setCenter(Vec3f center) { info.directionOrCenter = Vec4f(center, 1.f); }