#include "graphics/viewbuffer.h"
#include "graphics/graphics.h"
#include "graphics/gbuffer.h"
using namespace oi::gc;
using namespace oi;

ViewBuffer::~ViewBuffer() { g->destroy(info.buffer); }
ViewBuffer::ViewBuffer(ViewBufferInfo info) : info(info) {}
bool ViewBuffer::init() { return (info.buffer = g->create(getName() + " GBuffer", GBufferInfo(GBufferType::UBO, ViewBufferInfo::size))) != nullptr; }

GBuffer *ViewBuffer::getBuffer() { return info.buffer; }

void ViewBuffer::update() {

	bool updated = false;

	for (u32 i = 0; i < ViewBufferInfo::cameraCount; ++i) {

		if (info.cameras.isOccupied(i) && info.updated[getCameraId(i)]) {
			info.cameras[i].makeView();
			updated = true;
		}

	}

	for (u32 i = 0; i < ViewBufferInfo::frustumCount; ++i) {

		if (info.frusta.isOccupied(i) && info.updated[getFrustumId(i)]) {
			info.frusta[i].makeProjection();
			updated = true;
		}

	}

	for (u32 i = 0; i < ViewBufferInfo::viewCount; ++i) {

		ViewStruct &v = info.views[i];

		if (info.views.isOccupied(i) && (info.updated[getViewId(i)] || info.updated[getCameraId(v.camera)] || info.updated[getFrustumId(v.frustum)])) {
			v.makeViewProjection(this);
			updated = true;
		}

	}

	if (updated) {

		info.updated.clear();

		Buffer buf(ViewBufferInfo::size);

		u32 length, offset = 0;

		memcpy(buf.addr(), info.cameras.toBuffer().addr(), length = info.cameras.toBuffer().size()); offset += length;
		memcpy(buf.addr() + offset, info.frusta.toBuffer().addr(), length = info.frusta.toBuffer().size()); offset += length;
		memcpy(buf.addr() + offset, info.views.toBuffer().addr(), length = info.views.toBuffer().size()); offset += length;

		info.buffer->set(buf);
		buf.deconstruct();

	}

}

u32 ViewBuffer::getCameraId(CameraHandle cam) { return cam; }
u32 ViewBuffer::getFrustumId(CameraFrustumHandle vp) { return vp + ViewBufferInfo::cameraCount; }
u32 ViewBuffer::getViewId(ViewHandle v) { return v + ViewBufferInfo::cameraCount + ViewBufferInfo::frustumCount; }

void ViewBuffer::notify(u32 i) {
	info.updated[i] = true;
}

CameraStruct *ViewBuffer::alloc(const CameraStruct &cs) { CameraHandle handle = info.cameras.allocate(cs); notify(getCameraId(handle)); return &info.cameras[handle]; }
CameraFrustumStruct *ViewBuffer::alloc(const CameraFrustumStruct &vps) { CameraFrustumHandle handle = info.frusta.allocate(vps); notify(getFrustumId(handle)); return &info.frusta[handle]; }
ViewStruct *ViewBuffer::alloc(const ViewStruct &vs) { ViewHandle handle = info.views.allocate(vs); notify(getViewId(handle)); return &info.views[handle]; }

void ViewBuffer::dealloc(CameraStruct *cs) { info.cameras.dealloc(cs); }
void ViewBuffer::dealloc(CameraFrustumStruct *vps) { info.frusta.dealloc(vps); }
void ViewBuffer::dealloc(ViewStruct *vs) { info.views.dealloc(vs); }

CameraHandle ViewBuffer::get(CameraStruct *cs) { return info.cameras.find(cs); }
CameraFrustumHandle ViewBuffer::get(CameraFrustumStruct *vps) { return info.frusta.find(vps); }
ViewHandle ViewBuffer::get(ViewStruct *vs) { return info.views.find(vs); }

CameraStruct *ViewBuffer::getCamera(CameraHandle cam) { return &info.cameras[cam]; }
CameraFrustumStruct *ViewBuffer::getFrustum(CameraFrustumHandle vps) { return &info.frusta[vps]; }
ViewStruct *ViewBuffer::getView(ViewHandle vs) { return &info.views[vs]; }
