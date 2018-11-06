#pragma once

#include "camera.h"
#include "camerafrustum.h"

namespace oi {

	namespace gc {

		class ViewBuffer;
		typedef u32 ViewHandle;

		struct ViewStruct {

			CameraHandle camera;
			CameraFrustumHandle frustum;
			u32 p0 = 0, p1 = 0;

			Matrix vp;

			ViewStruct() { memset(this, 0, sizeof(ViewStruct)); }
			ViewStruct(CameraHandle camera, CameraFrustumHandle frustum) : camera(camera), frustum(frustum) { }

			void makeViewProjection(ViewBuffer *buffer);

		};

		class View;

		struct ViewInfo {

			typedef View ResourceType;

			ViewStruct temp;

			ViewBuffer *parent;
			ViewStruct *ptr;

			ViewInfo(ViewBuffer *parent, Camera *cam, CameraFrustum *cf) : parent(parent), temp(cam->getHandle(), cf->getHandle()), ptr(&temp) {
				if (cam->getParent() != parent || cf->getParent() != parent)
					Log::throwError<ViewInfo, 0x0>("Camera, Viewport and View aren't in the same ViewBuffer");
			}

		};

		class View : public GraphicsObject {

			friend class Graphics;
			friend class ViewBuffer;

		public:

			const ViewStruct &getStruct();

			void setCamera(Camera *cam);
			void setFrustum(CameraFrustum *frustum);

			ViewHandle getHandle();
			ViewBuffer *getParent();

		protected:

			~View();
			View(ViewInfo info);
			bool init();

		private:

			ViewInfo info;
			ViewHandle handle;

		};

	}

}