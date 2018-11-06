#pragma once

#include "memory/objectallocator.h"
#include "view.h"

namespace oi {

	namespace gc {

		class ViewBuffer;
		class GBuffer;

		struct ViewBufferInfo {

			typedef ViewBuffer ResourceType;

			static constexpr u32 cameraCount = 128, frustumCount = 128, viewCount = 256, 
				size = cameraCount * (u32) sizeof(CameraStruct) + frustumCount * (u32) sizeof(CameraFrustumStruct) + viewCount * (u32) sizeof(ViewStruct);

			StaticObjectAllocator<CameraStruct, cameraCount> cameras;
			StaticObjectAllocator<CameraFrustumStruct, frustumCount> frusta;
			StaticObjectAllocator<ViewStruct, viewCount> views;

			StaticBitset<cameraCount + frustumCount + viewCount> updated;

			GBuffer *buffer;

			ViewBufferInfo() { }

		};

		class ViewBuffer : public GraphicsObject {

			friend class Graphics;
			friend class Camera;
			friend class CameraFrustum;
			friend class View;

		public:

			void update();	//Update all camera, viewport and view matrices

			CameraStruct *alloc(const CameraStruct &cs);
			CameraFrustumStruct *alloc(const CameraFrustumStruct &vps);
			ViewStruct *alloc(const ViewStruct &vs);

			void dealloc(CameraStruct *cs);
			void dealloc(CameraFrustumStruct *vps);
			void dealloc(ViewStruct *vs);

			CameraHandle get(CameraStruct *cs);
			CameraFrustumHandle get(CameraFrustumStruct *vps);
			ViewHandle get(ViewStruct *vs);

			CameraStruct *getCamera(CameraHandle cam);
			CameraFrustumStruct *getFrustum(CameraFrustumHandle vp);
			ViewStruct *getView(ViewHandle v);

			GBuffer *getBuffer();

		protected:

			~ViewBuffer();
			ViewBuffer(ViewBufferInfo info);
			bool init();

			u32 getCameraId(CameraHandle cam);
			u32 getFrustumId(CameraFrustumHandle vp);
			u32 getViewId(ViewHandle v);

			void notify(u32 i);

		private:

			ViewBufferInfo info;

		};

	}

}