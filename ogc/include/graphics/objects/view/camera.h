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

			Vec3 rotation;
			f32 p1 = 0;

			CameraStruct();
			CameraStruct(Vec3 position, Vec3 rotation);

			void makeView();

		};

		class Camera;
		class ViewBuffer;

		struct CameraInfo {

			typedef Camera ResourceType;

			CameraStruct temp;

			ViewBuffer *parent;
			CameraStruct *ptr;

			CameraInfo(ViewBuffer *parent, Vec3 position = { }, Vec3 rotation = {}) : parent(parent), temp(position, rotation), ptr(&temp) {}

		};

		class Camera : public GraphicsObject {

			friend class Graphics;
			friend class ViewBuffer;
			friend class oi::BlockAllocator;

		public:

			const CameraStruct &getStruct();

			void move(Vec3 dposition);
			void moveLocal(Vec3 dposition);

			void rotate(Vec3 drotation);

			void setPosition(Vec3 position);
			void setRotation(Vec3 rotation);

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