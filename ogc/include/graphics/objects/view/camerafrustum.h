#pragma once

#include "types/vector.h"
#include "types/matrix.h"
#include "../graphicsobject.h"

#undef near
#undef far

namespace oi {

	namespace gc {

		typedef u32 CameraFrustumHandle;

		struct CameraFrustumStruct {

			Matrix p;

			f32 near, far, aspect, fov;

			Vec2u resolution;
			u32 p0 = 0, p1 = 0;

			CameraFrustumStruct() { memset(this, 0, sizeof(CameraFrustumStruct)); }
			CameraFrustumStruct(f32 near, f32 far, f32 aspect, Vec2u res, f32 fov) : far(far), near(near), aspect(aspect), resolution(res), fov(fov) { }

			void makeProjection() { p = Matrix::makePerspective(fov, aspect, near, far); }

		};

		class CameraFrustum;
		class ViewBuffer;

		struct CameraFrustumInfo {

			typedef CameraFrustum ResourceType;

			CameraFrustumStruct temp;

			ViewBuffer *parent;
			CameraFrustumStruct *ptr;

			CameraFrustumInfo(ViewBuffer *parent, Vec2u res, f32 aspect, f32 fov, f32 near, f32 far) : parent(parent), temp(near, far, aspect, res, fov), ptr(&temp) {}

		};

		class CameraFrustum : public GraphicsObject {

			friend class Graphics;
			friend class ViewBuffer;
			friend class oi::BlockAllocator;

		public:

			const CameraFrustumStruct &getStruct();

			void resize(Vec2u size);
			void setAspect(f32 aspect);
			void setNear(f32 near);
			void setFar(f32 far);
			void setFov(f32 fov);

			void resize(Vec2u size, f32 aspect);

			CameraFrustumHandle getHandle();
			ViewBuffer *getParent();

		protected:

			~CameraFrustum();
			CameraFrustum(CameraFrustumInfo info);
			bool init();

			void changed();

		private:

			CameraFrustumInfo info;
			CameraFrustumHandle handle;

		};

	}

}