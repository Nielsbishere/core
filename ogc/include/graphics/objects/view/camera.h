#pragma once

#include "types/vector.h"
#include "types/matrix.h"
#include "../graphicsobject.h"

namespace oi {

	namespace gc {

		typedef u32 CameraHandle;

		struct CameraStruct {

			Matrix v;

			Vec3 position;
			f32 p0 = 0;

			Vec3 up;
			f32 p1 = 0;

			Vec4 forward;

			CameraStruct() { memset(this, 0, sizeof(CameraStruct));  }
			CameraStruct(Vec3 position, Vec3 up, Vec4 forward) : position(position), up(up), forward(forward.normalize()) {}

			void makeView() { 
				Vec3 center = forward.w == 1 ? Vec3(forward) : position + Vec3(forward);
				v = Matrix::makeView(position, center, up); 
			}

		};

		class Camera;
		class ViewBuffer;

		struct CameraInfo {

			typedef Camera ResourceType;

			CameraStruct temp;

			ViewBuffer *parent;
			CameraStruct *ptr;

			CameraInfo(ViewBuffer *parent, Vec3 position = { }, Vec4 directionOrCenter = { 0, 0, -1, 0 }, Vec3 up = { 0, 1, 0 }) : parent(parent), temp(position, up, directionOrCenter), ptr(&temp) {}

		};

		class Camera : public GraphicsObject {

			friend class Graphics;
			friend class ViewBuffer;
			friend class oi::BlockAllocator;

		public:

			const CameraStruct &getStruct();

			void move(Vec3 dposition);
			void moveLocal(Vec3 dposition);

			void setPosition(Vec3 position);
			void setUp(Vec3 up);
			void setDirection(Vec3 direction);
			void setCenter(Vec3 center);

			CameraHandle getHandle();
			ViewBuffer *getParent();

		protected:

			~Camera();
			Camera(CameraInfo info);
			bool init();

			void changed();

		private:

			CameraInfo info;
			CameraHandle handle;

		};

	}

}